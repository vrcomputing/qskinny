#include "QskColorRectangleSkinlet.h"
#include "QskColorRectangle.h"

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>

#include <QskSGNode.h>
#include <qmath.h>

using Q = QskColorRectangle;

namespace
{
    class QskVertexColoredRectangle : public QSGGeometryNode
    {
      public:
        QskVertexColoredRectangle( const QColor& topLeft = {}, const QColor& topRight = {},
            const QColor& bottomLeft = {}, const QColor& bottomRight = {} )
        {
            auto* const geometry =
                new QSGGeometry( QSGGeometry::defaultAttributes_ColoredPoint2D(), 4 );
            geometry->setDrawingMode( QSGGeometry::DrawTriangleStrip );
            setGeometry( geometry );

            auto* const material = new QSGVertexColorMaterial;
            setMaterial( material );

            update( {}, topLeft, topRight, bottomLeft, bottomRight );
        }

        void update( const QRectF& rect, const QColor& topLeft = {}, const QColor& topRight = {},
            const QColor& bottomLeft = {}, const QColor& bottomRight = {} )
        {
            auto* const vertices = geometry()->vertexDataAsColoredPoint2D();
            vertices[ 0 ].set( rect.left(), rect.top(), topLeft.red(), topLeft.green(),
                topLeft.blue(), topLeft.alpha() );
            vertices[ 1 ].set( rect.right(), rect.top(), topRight.red(), topRight.green(),
                topRight.blue(), topRight.alpha() );
            vertices[ 2 ].set( rect.left(), rect.bottom(), bottomLeft.red(), bottomLeft.green(),
                bottomLeft.blue(), bottomLeft.alpha() );
            vertices[ 3 ].set( rect.right(), rect.bottom(), bottomRight.red(), bottomRight.green(),
                bottomRight.blue(), bottomRight.alpha() );

            markDirty( QSGNode::DirtyGeometry );
            markDirty( QSGNode::DirtyMaterial );
        }
    };
}

QskColorRectangleSkinlet::QskColorRectangleSkinlet( QskSkin* skin )
    : QskSkinlet( skin )
{
    setNodeRoles( { PanelRole, HandleRole } );
}

QRectF QskColorRectangleSkinlet::subControlRect( const QskSkinnable* skinnable,
    const QRectF& contentsRect, QskAspect::Subcontrol subcontrol ) const
{
    const auto* const q = static_cast< const Q* >( skinnable );
    if ( subcontrol == Q::Panel )
    {
        return contentsRect;
    }
    if ( subcontrol == Q::Handle )
    {
        const auto size = q->strutSizeHint( Q::Handle );
        const auto w = qMax( 0.0, size.width() ) / 2;
        const auto h = qMax( 0.0, size.height() ) / 2;

        auto p = q->effectiveSkinHint( Q::Handle ).toPointF();
        p.rx() *= contentsRect.width();
        p.ry() *= contentsRect.height();
        p.rx() += contentsRect.left();
        p.ry() += contentsRect.top();

        return { QPointF{ p.x() - w, p.y() - h }, QPointF{ p.x() + w, p.y() + h } };
    }
    return QskSkinlet::subControlRect( skinnable, contentsRect, subcontrol );
}

QSGNode* QskColorRectangleSkinlet::updateSubNode(
    const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* node ) const
{
    const auto* const q = static_cast< const Q* >( skinnable );

    if ( nodeRole == PanelRole )
    {
        auto* const rectangleNode = QskSGNode::ensureNode< QskVertexColoredRectangle >( node );
        rectangleNode->update( q->subControlRect( Q::Panel ), q->cornerColor( Qt::TopLeftCorner ),
            q->cornerColor( Qt::TopRightCorner ), q->cornerColor( Qt::BottomLeftCorner ),
            q->cornerColor( Qt::BottomRightCorner ) );
        return rectangleNode;
    }

    if ( nodeRole == HandleRole )
    {
        return updateBoxNode( skinnable, node, Q::Handle );
    }

    return QskSkinlet::updateSubNode( skinnable, nodeRole, node );
}
