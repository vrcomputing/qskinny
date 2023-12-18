#include "ColorPickerPage.h"
#include "QskAspect.h"
#include "QskBoxNode.h"
#include "QskControl.h"
#include "QskGradient.h"
#include "QskGradientStop.h"
#include "QskGridBox.h"
#include "QskLinearBox.h"
#include "QskSkin.h"
#include "QskSkinlet.h"
#include "QskSkinnable.h"
#include "QskSlider.h"
#include "QskTextLabel.h"
#include "colorpickers/ColorPickerPage.h"
#include <QskArcMetrics.h>
#include <QskArcNode.h>
#include <QskBoxBorderColors.h>
#include <QskBoxBorderMetrics.h>
#include <QskBoxShapeMetrics.h>
#include <QskGridBox.h>

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGSimpleMaterialShader>
#include <QSGVertexColorMaterial>

#include <QQuickWindow>
#include <QskSGNode.h>
#include <cmath>
#include <qcolor.h>
#include <qmath.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qpoint.h>
#include <qsggeometry.h>
#include <qsgnode.h>
#include <qtransform.h>
#include <qvector2d.h>
#include <qvector3d.h>

namespace
{
    double qskMapValueRange(
        double value, double srcMin, double srcMax, double dstMin, double dstMax )
    {
        value = std::min( std::max( value, srcMin ), srcMax );
        double percentage = ( value - srcMin ) / ( srcMax - srcMin );
        double mappedValue = dstMin + percentage * ( dstMax - dstMin );
        return mappedValue;
    }

    QVector3D cartesian2barycentric(
        const QPointF& p, const QPointF& t1, const QPointF& t2, const QPointF& t3 )
    {
        const auto x = p.x();
        const auto y = p.y();

        const auto x1 = t1.x();
        const auto y1 = t1.y();
        const auto x2 = t2.x();
        const auto y2 = t2.y();
        const auto x3 = t3.x();
        const auto y3 = t3.y();

        const auto y2y3 = y2 - y3, x3x2 = x3 - x2, x1x3 = x1 - x3, y1y3 = y1 - y3, y3y1 = y3 - y1,
                   xx3 = x - x3, yy3 = y - y3;

        const auto d = y2y3 * x1x3 + x3x2 * y1y3, lambda1 = ( y2y3 * xx3 + x3x2 * yy3 ) / d,
                   lambda2 = ( y3y1 * xx3 + x1x3 * yy3 ) / d;

        return { ( float ) lambda1, ( float ) lambda2, ( float ) ( 1.0 - lambda1 - lambda2 ) };
    }

    QColor qskColorInTriangle( const QPointF& p, const QPointF& t1, const QPointF& t2,
        const QPointF& t3, const QColor& c1, const QColor& c2, const QColor& c3 )
    {
        const auto t = cartesian2barycentric( p, t1, t2, t3 );
        const auto r =
            std::floor( ( c1.redF() * t[ 0 ] + c2.redF() * t[ 1 ] + c3.redF() * t[ 2 ] ) * 255.0 );
        const auto g = std::floor(
            ( c1.greenF() * t[ 0 ] + c2.greenF() * t[ 1 ] + c3.greenF() * t[ 2 ] ) * 255.0 );
        const auto b = std::floor(
            ( c1.blueF() * t[ 0 ] + c2.blueF() * t[ 1 ] + c3.blueF() * t[ 2 ] ) * 255.0 );

        if ( t[ 0 ] < 0.0 || t[ 1 ] < 0.0 || t[ 2 ] < 0.0 )
        {
            return {};
        }

        return { int( r ), int( g ), int( b ) };
    }

    QColor qskColorAt( const QskGradient& gradient, qreal ratio )
    {
        return gradient.extracted( ratio, ratio ).startColor();
    }

    QColor qskColorForAngle( const QskGradient& gradient, qreal angle )
    {
        angle = ( angle < 0.0 ? angle + 2 * M_PI : angle );
        const auto ratio = qAbs( angle / ( 2 * M_PI ) );
        return qskColorAt( gradient, ratio );
    }
}

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

class QskColorTriangleNode : public QSGGeometryNode
{
    static constexpr auto vertexCount = 3;

  public:
    QskColorTriangleNode()
    {
        auto* const geometry =
            new QSGGeometry( QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount );
        geometry->setDrawingMode( QSGGeometry::DrawTriangles );
        setGeometry( geometry );

        auto* const material = new QSGVertexColorMaterial;
        setMaterial( material );
    }

    void update(
        const QRectF& rect, const QColor& c1, const QColor& c2, const QColor& c3, const qreal rad )
    {
        const QColor colors[]{ c1, c2, c3 };

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

QSK_SUBCONTROL( QskColorWheel, Arc )
QSK_SUBCONTROL( QskColorWheel, Handle )

QskColorWheel::QskColorWheel( QQuickItem* parent )
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

    setColor( Handle, Qt::white );
    setBoxBorderColorsHint( Handle, { Qt::white } );
    setBoxBorderMetricsHint( Handle, { 2 } );
    setBoxShapeHint( Handle, { 100.0, 100.0, 100.0, 100.0, Qt::RelativeSize } );
}

QColor QskColorWheel::selectedColor() const
{
    return m_selectedColor;
}

void QskColorWheel::setSelectedColor( const QColor& color )
{
    if ( m_selectedColor != color )
    {
        m_selectedColor = color;
        setColor( Handle, color );
        Q_EMIT selectedColorChanged( m_selectedColor );
    }
}

void QskColorWheel::mousePressEvent( QMouseEvent* event )
{
}

void QskColorWheel::mouseReleaseEvent( QMouseEvent* event )
{
}

void QskColorWheel::mouseMoveEvent( QMouseEvent* event )
{
    const auto r = contentsRect();
    const auto ux = qskMapValueRange( event->pos().x(), r.left(), r.right(), -1, +1 );
    const auto uy = qskMapValueRange( event->pos().y(), r.top(), r.bottom(), +1, -1 );

    if ( QVector2D( ux, uy ).length() > 1.0 ) // TODO && Pressed
    {
        event->ignore();
        return;
    }

    const auto rad = qAtan2( uy, ux );
    const auto color = qskColorForAngle( gradientHint( Arc ), rad );
    setPositionHint( Handle, -rad );
    setSelectedColor( color );
}

QskColorWheelSkinlet::QskColorWheelSkinlet( QskSkin* skin )
    : QskSkinlet( skin )
{
    setNodeRoles( { ArcRole, HandleRole } );
}

QRectF QskColorWheelSkinlet::subControlRect( const QskSkinnable* skinnable,
    const QRectF& contentsRect, QskAspect::Subcontrol subcontrol ) const
{
    using Q = QskColorWheel;
    const auto* const q = static_cast< const Q* >( skinnable );

    if ( subcontrol == Q::Arc )
    {
        const auto size = qMin( contentsRect.width(), contentsRect.height() );
        return { contentsRect.center().x() - size / 2, contentsRect.center().y() - size / 2, size,
            size };
    }

    if ( subcontrol == Q::Handle )
    {
        const auto& r = contentsRect;
        const auto thickness = skinnable->arcMetricsHint( Q::Arc ).thickness() / 2;
        const auto angle = q->positionHint( Q::Handle );
        const auto radius = qMin( r.width(), r.height() ) / 2 - thickness;
        const auto x = contentsRect.center().x() + radius * qFastCos( angle );
        const auto y = contentsRect.center().y() + radius * qFastSin( angle );
        return { QPointF{ x - thickness, y - thickness }, QPointF{ x + thickness, y + thickness } };
    }

    return QskSkinlet::subControlRect( skinnable, contentsRect, subcontrol );
}

QSGNode* QskColorWheelSkinlet::updateSubNode(
    const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* node ) const
{
    using Q = QskColorWheel;
    const auto* const q = static_cast< const Q* >( skinnable );

    if ( nodeRole == ArcRole )
    {
        return updateArcNode( skinnable, node, Q::Arc );
    }

    if ( nodeRole == HandleRole )
    {
        return updateBoxNode( skinnable, node, Q::Handle );
    }

    return QskSkinlet::updateSubNode( skinnable, nodeRole, node );
}

QSizeF QskColorWheelSkinlet::sizeHint(
    const QskSkinnable* skinnable, Qt::SizeHint which, const QSizeF& ) const
{
    if ( which != Qt::PreferredSize )
    {
        return {};
    }

    const auto* const q = static_cast< const QskColorTriangle* >( skinnable );
    const auto rect = q->contentsRect();
    const auto size = qMin( rect.width(), rect.height() );
    return { size, size };
}

ColorPickerPage::ColorPickerPage( QQuickItem* parent )
    : Page( parent )
{
    auto* const layout = new QskGridBox( this );
    auto* const colorWheel = new QskColorWheel( layout );
    {
        auto* const skinlet = new QskColorWheelSkinlet;
        colorWheel->setSkinlet( skinlet );
        skinlet->setOwnedBySkinnable( true );
    }
    auto* const colorTriangle = new QskColorTriangle( layout );
    {
        auto* const skinlet = new QskColorTriangleSkinlet;
        colorTriangle->setSkinlet( skinlet );
        skinlet->setOwnedBySkinnable( true );
    }

    layout->addItem( new QskTextLabel( "QskColorWheel" ), 0, 0 );
    layout->addItem( colorWheel, 1, 0 );
    layout->addItem( new QskTextLabel( "QskColorTriangle" ), 0, 1 );
    layout->addItem( colorTriangle, 1, 1 );

    for ( int col = 0; col < layout->columnCount(); col++ )
    {
        layout->setColumnStretchFactor( col, 1 );
    }

    connect( colorWheel, &QskColorWheel::selectedColorChanged, colorTriangle,
        [ colorTriangle ](
            const QColor& color ) { colorTriangle->setColors( color, Qt::black, Qt::white ); } );
}

QSK_SUBCONTROL( QskColorTriangle, Triangle )
QSK_SUBCONTROL( QskColorTriangle, Handle )

QskColorTriangle::QskColorTriangle( QQuickItem* parent )
    : QskControl( parent )
{
    setAcceptedMouseButtons( Qt::LeftButton );

    setStrutSizeHint( Handle, strutSizeHint( QskSlider::Handle ) );
    setColor( Handle, Qt::white );
    setBoxBorderColorsHint( Handle, { Qt::white } );
    setBoxBorderMetricsHint( Handle, { 2 } );
    setBoxShapeHint( Handle, { 100.0, 100.0, 100.0, 100.0, Qt::RelativeSize } );

    setPositionHint( Triangle, qDegreesToRadians( 45.0 ) );
}

QColor QskColorTriangle::selectedColor() const
{
    return m_selectedColor;
}

void QskColorTriangle::setSelectedColor( const QColor& color )
{
    if ( m_selectedColor != color )
    {
        m_selectedColor = color;
        setColor( Handle, color );
        Q_EMIT selectedColorChanged( m_selectedColor );
    }
}

void QskColorTriangle::setColors( const QColor& c1, const QColor& c2, const QColor& c3 )
{
    m_c1 = c1;
    m_c2 = c2;
    m_c3 = c3;
    update();
}

QColor QskColorTriangle::color1() const
{
    return m_c1;
}

QColor QskColorTriangle::color2() const
{
    return m_c2;
}

QColor QskColorTriangle::color3() const
{
    return m_c3;
}

void QskColorTriangle::mousePressEvent( QMouseEvent* event )
{
}

void QskColorTriangle::mouseReleaseEvent( QMouseEvent* event )
{
}

void QskColorTriangle::mouseMoveEvent( QMouseEvent* event )
{
    const auto rect = subControlRect( Triangle );
    if ( rect.contains( event->pos() ) )
    {
        static constexpr auto step = 2 * M_PI / 3;
        const auto angle = 0.0;
        const auto radius = qMin( rect.width(), rect.height() ) / 2;
        QTransform transformation;
        transformation.rotate( angle, Qt::ZAxis );

        auto t1 = QPointF{ qFastCos( 0 * step ), qFastSin( 0 * step ) };
        auto t2 = QPointF{ qFastCos( 1 * step ), qFastSin( 1 * step ) };
        auto t3 = QPointF{ qFastCos( 2 * step ), qFastSin( 2 * step ) };

        t1 *= radius;
        t2 *= radius;
        t3 *= radius;

        t1 += rect.center();
        t2 += rect.center();
        t3 += rect.center();

        const auto color = qskColorInTriangle( event->pos(), t1, t2, t3, m_c1, m_c2, m_c3 );
        if ( color.isValid() )
        {
            setSelectedColor( color );
            setSkinHint( Handle, event->pos() );
        }
    }
    event->ignore();
}

QskColorTriangleSkinlet::QskColorTriangleSkinlet( QskSkin* skin )
    : QskSkinlet( skin )
{
    setNodeRoles( { TriangleRole, HandleRole } );
}

QRectF QskColorTriangleSkinlet::subControlRect( const QskSkinnable* skinnable,
    const QRectF& contentsRect, QskAspect::Subcontrol subcontrol ) const
{
    using Q = QskColorTriangle;
    const auto* const q = static_cast< const Q* >( skinnable );

    const auto& r = contentsRect;

    if ( subcontrol == Q::Triangle )
    {
        const auto size = ( qMin( r.width(), contentsRect.height() ) ) / 2;
        auto rect = QRectF( 0, 0, size, size );

        return { QPointF{ r.center().x() - size, r.center().y() - size },
            QPointF{ r.center().x() + size, r.center().y() + size } };
    }

    if ( subcontrol == Q::Handle )
    {
        const auto rect = subControlRect( skinnable, contentsRect, Q::Triangle );
        const auto size =
            qMin( q->strutSizeHint( Q::Handle ).width(), q->strutSizeHint( Q::Handle ).height() );
        const auto position = q->effectiveSkinHint( Q::Handle ).toPointF();
        return { QPointF{ position.x() - size, position.y() - size },
            QPointF{ position.x() + size, position.y() + size } };
    }
    return QskSkinlet::subControlRect( skinnable, contentsRect, subcontrol );
}

QSGNode* QskColorTriangleSkinlet::updateSubNode(
    const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* node ) const
{
    using Q = QskColorTriangle;
    const auto* const q = static_cast< const Q* >( skinnable );

    if ( nodeRole == TriangleRole )
    {
        const auto subcontrol = Q::Triangle;
        const auto rect = subControlRect( skinnable, q->contentsRect(), subcontrol );
        auto* const triangleNode = QskSGNode::ensureNode< QskColorTriangleNode >( node );
        triangleNode->update(
            rect, q->color1(), q->color2(), q->color3(), q->positionHint( Q::Handle ) );
        return triangleNode;
    }
    if ( nodeRole == HandleRole )
    {
        return updateBoxNode( skinnable, node, Q::Handle );
    }

    return QskSkinlet::updateSubNode( skinnable, nodeRole, node );
}

QSizeF QskColorTriangleSkinlet::sizeHint(
    const QskSkinnable* skinnable, Qt::SizeHint which, const QSizeF& ) const
{
    if ( which != Qt::PreferredSize )
    {
        return {};
    }

    const auto* const q = static_cast< const QskColorTriangle* >( skinnable );
    const auto rect = q->contentsRect();
    const auto size = qMin( rect.width(), rect.height() );
    return { size, size };
}