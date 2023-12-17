#include "ColorPickerPage.h"
#include "QskBoxNode.h"
#include "QskControl.h"
#include "QskGradient.h"
#include "QskGradientStop.h"
#include "QskLinearBox.h"
#include "QskSkin.h"
#include "QskSkinlet.h"
#include "QskSkinnable.h"
#include "QskSlider.h"
#include "colorpickers/ColorPickerPage.h"
#include <QskArcMetrics.h>
#include <QskArcNode.h>

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGSimpleMaterialShader>
#include <QSGVertexColorMaterial>

#include <QQuickWindow>
#include <QskSGNode.h>
#include <qmath.h>
#include <qnamespace.h>
#include <qpoint.h>
#include <qsggeometry.h>
#include <qsgnode.h>

struct Color
{
    float r, g, b, a;
    QRectF rect;
    QPointF mouse;
};

class MinimalShader : public QSGSimpleMaterialShader< Color >
{
    QSG_DECLARE_SIMPLE_SHADER( MinimalShader, Color )
  public:
    MinimalShader()
    {
        setShaderSourceFile( QOpenGLShader::Fragment, ":/qskinny/shaders/colorwheel.frag" );
        setShaderSourceFile( QOpenGLShader::Vertex, ":/qskinny/shaders/colorwheel.vert" );
    }

    QList< QByteArray > attributes() const
    {
        return QList< QByteArray >() << "vertex";
    }

    void updateState( const Color* color, const Color* )
    {
        program()->setUniformValue( "color", color->r, color->g, color->b, color->a );
        program()->setUniformValue(
            "rect", color->rect.x(), color->rect.y(), color->rect.width(), color->rect.height() );
        program()->setUniformValue(
            "mouse", color->mouse.x(), color->mouse.y(), color->mouse.x(), color->mouse.y() );
    }
};

class QskBrightnessTriangleNode : public QSGGeometryNode
{
    static constexpr auto vertexCount = 3;

  public:
    QskBrightnessTriangleNode()
    {
        auto* const geometry =
            new QSGGeometry( QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount );
        geometry->setDrawingMode( QSGGeometry::DrawTriangles );
        setGeometry( geometry );

        auto* const material = new QSGVertexColorMaterial;
        setMaterial( material );
    }

    void update( const QRectF& rect, const QColor& color, const qreal rad )
    {
        const QColor colors[]{ color, Qt::black, Qt::white };

        const auto center = rect.center();
        const auto radius = qMin( rect.width(), rect.height() ) / 2;

        const auto step = 2 * M_PI / 3;

        auto* const vertices = geometry()->vertexDataAsColoredPoint2D();
        for ( int i = 0; i < vertexCount; ++i )
        {
            const auto x = qFastCos( rad + i * step ) * radius + center.x();
            const auto y = qFastSin( rad + i * step ) * radius + center.y();
            const uchar c[ 4 ]{ ( uchar ) colors[ i ].red(), ( uchar ) colors[ i ].green(),
                ( uchar ) colors[ i ].blue(), 255 };
            vertices[ i ].set( x, y, c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ] );
        }
        geometry()->markVertexDataDirty();
        markDirty( QSGNode::DirtyGeometry );
        markDirty( QSGNode::DirtyMaterial );
    }
};

class QskColorWheelNode : public QSGGeometryNode
{
    using Material = QSGSimpleMaterial< Color >;

  public:
    QskColorWheelNode()
    {
        auto* const geometry =
            new QSGGeometry( QSGGeometry::defaultAttributes_TexturedPoint2D(), 4 );
        geometry->setDrawingMode( QSGGeometry::DrawTriangleStrip );
        setGeometry( geometry );
        setMaterial( MinimalShader::createMaterial() );
        material()->setFlag( QSGMaterial::Blending );
    }

    void update( const QRectF& rect, const QPointF& mouse, const QRectF& rectOnScreen,
        const QSizeF& resolution )
    {
        auto* const material = static_cast< Material* >( this->material() );
        material->state()->r = 1.0;
        material->state()->g = 0.0;
        material->state()->b = 1.0;
        material->state()->a = 1.0;
        material->state()->rect = { rectOnScreen.x(), resolution.height() - rectOnScreen.bottom(),
            rectOnScreen.width(), rectOnScreen.height() };
        material->state()->mouse = { mouse.x(), resolution.height() - mouse.y() };

        QSGGeometry::updateTexturedRectGeometry( geometry(), rect, { -0.5, -0.5, 1.0, 1.0 } );
        geometry()->markVertexDataDirty();
        markDirty( QSGNode::DirtyGeometry ); // ?
        markDirty( QSGNode::DirtyMaterial ); // ?
    }

  private:
};

QSK_SUBCONTROL( QskColorWheel, Panel )
QSK_SUBCONTROL( QskColorWheel, Handle )

QskColorWheel::QskColorWheel( QQuickItem* parent )
    : QskControl( parent )
{
    setAcceptedMouseButtons( Qt::LeftButton );
}

void QskColorWheel::mousePressEvent( QMouseEvent* event )
{
}

void QskColorWheel::mouseReleaseEvent( QMouseEvent* event )
{
}

void QskColorWheel::mouseMoveEvent( QMouseEvent* event )
{
    setSkinHint( Handle, event->pos() );
}

QskColorWheelSkinlet::QskColorWheelSkinlet( QskSkin* skin )
    : QskSkinlet( skin )
{
    setNodeRoles( { PanelRole } );
}

QRectF QskColorWheelSkinlet::subControlRect(
    const QskSkinnable*, const QRectF& contentsRect, QskAspect::Subcontrol ) const
{
    return contentsRect;
}

QSGNode* QskColorWheelSkinlet::updateSubNode(
    const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* node ) const
{
    const auto* const q = static_cast< const QskColorWheel* >( skinnable );

    if ( nodeRole == PanelRole )
    {
        auto* const wheelNode = QskSGNode::ensureNode< QskColorWheelNode >( node );
        auto rect = q->contentsRect();
        const auto size = qMin( rect.width(), rect.height() );
        rect.setSize( { size, size } );
        const auto mouse = q->effectiveSkinHint( QskColorWheel::Handle ).toPointF();

        wheelNode->update(
            rect, q->mapToScene( mouse ), q->mapRectToScene( rect ), q->window()->size() );
        return wheelNode;
    }
    return QskSkinlet::updateSubNode( skinnable, nodeRole, node );
}

QSizeF QskColorWheelSkinlet::sizeHint(
    const QskSkinnable* skinnable, Qt::SizeHint which, const QSizeF& ) const
{
    if ( which != Qt::PreferredSize )
        return QSizeF();

    const auto* const q = static_cast< const QskColorWheel* >( skinnable );
    const auto size = q->contentsRect();
    return { qMin( size.width(), size.height() ), qMin( size.width(), size.height() ) };
}

ColorPickerPage::ColorPickerPage( QQuickItem* parent )
    : Page( parent )
{
    auto* const layout = new QskLinearBox( Qt::Horizontal, this );
    {
        auto* const control = new QskColorWheel( layout );
        auto* const skinlet = new QskColorWheelSkinlet;
        control->setSkinlet( skinlet );
        skinlet->setOwnedBySkinnable( true );
    }
    {
        auto* const row = new QskLinearBox( Qt::Vertical, layout );
        auto* const control = new QskBrightnessTriangle( row );
        auto* const skinlet = new QskBrightnessTriangleSkinlet;
        control->setSkinlet( skinlet );
        skinlet->setOwnedBySkinnable( true );

        auto* const thickness = new QskSlider( Qt::Horizontal, row );

        connect( thickness, &QskSlider::valueChanged, this, [ = ]( qreal v ) {
            auto metrics = control->arcMetricsHint( QskBrightnessTriangle::Arc );

            metrics.setThickness(
                control->subControlRect( QskBrightnessTriangle::Arc ).width() / 2 * v );
            control->setArcMetricsHint( QskBrightnessTriangle::Arc, metrics );
        } );
    }
}

QSK_SUBCONTROL( QskBrightnessTriangle, Panel )
QSK_SUBCONTROL( QskBrightnessTriangle, Handle )
QSK_SUBCONTROL( QskBrightnessTriangle, Arc )
QSK_SUBCONTROL( QskBrightnessTriangle, Triangle )

QskBrightnessTriangle::QskBrightnessTriangle( QQuickItem* parent )
    : QskControl( parent )
{
    setAcceptedMouseButtons( Qt::LeftButton );

    static const QskGradientStops stops = {
        { 0.0000, QColor::fromRgb( 255, 0, 0 ) },
        { 0.1667, QColor::fromRgb( 255, 255, 0 ) },
        { 0.3333, QColor::fromRgb( 0, 255, 0 ) },
        { 0.5000, QColor::fromRgb( 0, 255, 255 ) },
        { 0.6667, QColor::fromRgb( 0, 0, 255 ) },
        { 0.8333, QColor::fromRgb( 255, 0, 255 ) },
        { 1.0000, QColor::fromRgb( 255, 0, 0 ) },
    };

    QskGradient gradient( stops );
    setGradientHint( Arc, gradient );
    setArcMetricsHint( Arc, { 0, 360, 16 } );
}

void QskBrightnessTriangle::mousePressEvent( QMouseEvent* event )
{
}

void QskBrightnessTriangle::mouseReleaseEvent( QMouseEvent* event )
{
}

QColor getColorForPolarCoordinates( double angle )
{
    angle = ( angle < 0.0 ? angle + 2 * M_PI : angle );

    static const QskGradientStops stopsRGB = {
        { 0.0000, QColor::fromRgb( 255, 0, 0 ) },
        { 0.1667, QColor::fromRgb( 255, 255, 0 ) },
        { 0.3333, QColor::fromRgb( 0, 255, 0 ) },
        { 0.5000, QColor::fromRgb( 0, 255, 255 ) },
        { 0.6667, QColor::fromRgb( 0, 0, 255 ) },
        { 0.8333, QColor::fromRgb( 255, 0, 255 ) },
        { 1.0000, QColor::fromRgb( 255, 0, 0 ) },
    };

    static const QskGradient gradient( stopsRGB );
    const auto p = qAbs( angle / ( 2 * M_PI ) );
    return gradient.extracted( p, p ).startColor();
}

void QskBrightnessTriangle::mouseMoveEvent( QMouseEvent* event )
{
    auto r = contentsRect();
    const auto x = event->pos().x() * 2 - r.width();
    const auto y = event->pos().y() * 2 - r.height();
    setPositionHint( Handle, qAtan2( y, x ) );
    setColor( Handle, getColorForPolarCoordinates( -qAtan2( y, x ) ) );
}

QskBrightnessTriangleSkinlet::QskBrightnessTriangleSkinlet( QskSkin* skin )
    : QskSkinlet( skin )
{
    setNodeRoles( { ArcRole, TriangleRole } );
}

QRectF QskBrightnessTriangleSkinlet::subControlRect( const QskSkinnable* skinnable,
    const QRectF& contentsRect, QskAspect::Subcontrol subcontrol ) const
{
    const auto& r = contentsRect;

    if ( subcontrol == QskBrightnessTriangle::Arc )
    {
        const auto size = qMin( r.width(), r.height() ) / 2;
        auto rect = QRectF( 0, 0, size, size );

        return { QPointF{ r.center().x() - size, r.center().y() - size },
            QPointF{ r.center().x() + size, r.center().y() + size } };
    }
    if ( subcontrol == QskBrightnessTriangle::Triangle )
    {
        const auto thickness = skinnable->arcMetricsHint( QskBrightnessTriangle::Arc ).thickness();
        const auto size = ( qMin( r.width(), contentsRect.height() ) ) / 2 - thickness;
        auto rect = QRectF( 0, 0, size, size );

        return { QPointF{ r.center().x() - size, r.center().y() - size },
            QPointF{ r.center().x() + size, r.center().y() + size } };
    }
    return QskSkinlet::subControlRect( skinnable, contentsRect, subcontrol );
}

QSGNode* QskBrightnessTriangleSkinlet::updateSubNode(
    const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* node ) const
{
    using Q = QskBrightnessTriangle;
    const auto* const q = static_cast< const Q* >( skinnable );
    if ( nodeRole == ArcRole )
    {
        const auto subcontrol = Q::Arc;
        auto* const arcNode = QskSGNode::ensureNode< QskArcNode >( node );
        const auto rect = subControlRect( skinnable, q->contentsRect(), subcontrol );
        arcNode->setArcData( rect, q->arcMetricsHint( subcontrol ), q->gradientHint( subcontrol ) );
        return arcNode;
    }
    if ( nodeRole == TriangleRole )
    {
        const auto subcontrol = Q::Triangle;
        const auto rect = subControlRect( skinnable, q->contentsRect(), subcontrol );
        auto* const triangleNode = QskSGNode::ensureNode< QskBrightnessTriangleNode >( node );
        triangleNode->update( rect, q->color( Q::Handle ), q->positionHint( Q::Handle ) );
        return triangleNode;
    }
    return QskSkinlet::updateSubNode( skinnable, nodeRole, node );
}

QSizeF QskBrightnessTriangleSkinlet::sizeHint(
    const QskSkinnable* skinnable, Qt::SizeHint which, const QSizeF& ) const
{
    if ( which != Qt::PreferredSize )
        return QSizeF();

    const auto* const q = static_cast< const QskBrightnessTriangle* >( skinnable );
    const auto rect = q->contentsRect();
    const auto size = qMin( rect.width(), rect.height() );
    return { size, size };
}