#include "QskColorTriangleSkinlet.h"
#include "QskColorTriangle.h"
#include "QskColorTriangleNode.h"
#include <QskSGNode.h>
#include <qmath.h>

using Q = QskColorTriangle;

QskColorTriangleSkinlet::QskColorTriangleSkinlet( QskSkin* skin )
    : QskSkinlet( skin )
{
    setNodeRoles( { TriangleRole, HandleRole } );
}

QRectF QskColorTriangleSkinlet::subControlRect( const QskSkinnable* skinnable,
    const QRectF& contentsRect, QskAspect::Subcontrol subcontrol ) const
{
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
            qMin( q->strutSizeHint( Q::Handle ).width(), q->strutSizeHint( Q::Handle ).height() ) / 2;
        const auto position = q->effectiveSkinHint( Q::Handle ).toPointF();
        const auto radius = qMin( rect.width(), rect.height() ) / 2;

        if ( position == QPoint{} )
        {
            return {};
        }

        QTransform t;
        t.rotate( qRadiansToDegrees( q->positionHint( Q::Triangle ) ), Qt::ZAxis );
        const auto c = t.map( QPointF{ position.x(), -1 * position.y() } ) * radius + rect.center();

        return { QPointF{ c.x() - size, c.y() - size }, QPointF{ c.x() + size, c.y() + size } };
    }
    return QskSkinlet::subControlRect( skinnable, contentsRect, subcontrol );
}

QSGNode* QskColorTriangleSkinlet::updateSubNode(
    const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* node ) const
{
    const auto* const q = static_cast< const Q* >( skinnable );

    if ( nodeRole == TriangleRole )
    {
        const auto subcontrol = Q::Triangle;
        const auto rect = subControlRect( skinnable, q->contentsRect(), subcontrol );
        auto* const triangleNode = QskSGNode::ensureNode< QskColorTriangleNode >( node );
        triangleNode->update(
            rect, q->color1(), q->color2(), q->color3(), q->positionHint( Q::Triangle ) );
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