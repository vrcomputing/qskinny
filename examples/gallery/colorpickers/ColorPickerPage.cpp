#include "ColorPickerPage.h"
#include "QskLinearBox.h"
#include "QskSkinlet.h"
#include "colorpickers/ColorPickerPage.h"

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGSimpleMaterialShader>

#include <QQuickWindow>
#include <QskSGNode.h>
#include <qnamespace.h>
#include <qpoint.h>

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
        qDebug() << color->mouse;
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


        wheelNode->update( rect, q->mapToScene(mouse), q->mapRectToScene( rect ), q->window()->size() );
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
    auto* const layout = new QskLinearBox( this );
    auto* const control = new QskColorWheel( layout );
    auto* const skinlet = new QskColorWheelSkinlet;
    control->setSkinlet( skinlet );
    skinlet->setOwnedBySkinnable( true );
}