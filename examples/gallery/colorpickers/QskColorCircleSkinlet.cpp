#include "QskColorCircleSkinlet.h"
#include "QskCircleNode.h"
#include "QskColorCircle.h"
#include "QskSkinlet.h"
#include "colorpickers/QskCircleNode.h"
#include "colorpickers/QskColorCircleSkinlet.h"

#include <QskSGNode.h>
#include <qpoint.h>

using Q = QskColorCircle;

QskColorCircleSkinlet::QskColorCircleSkinlet( QskSkin* skin )
    : QskSkinlet( skin )
{
    setNodeRoles( { CircleRole, HandleRole } );
}

QRectF QskColorCircleSkinlet::subControlRect( const QskSkinnable* skinnable,
    const QRectF& contentsRect, QskAspect::Subcontrol subcontrol ) const
{
    const auto* const q = static_cast< const Q* >( skinnable );

    if ( subcontrol == Q::Circle )
    {
        const auto c = contentsRect.center();
        const auto r = qMin( contentsRect.width(), contentsRect.height() ) / 2;
        return { QPointF{ c.x() - r, c.y() - r }, QPointF{ c.x() + r, c.y() + r } };
    }

    if ( subcontrol == Q::Handle )
    {
        const auto c = contentsRect.center();
        const auto r = qMin( contentsRect.width(), contentsRect.height() ) / 2;
        const auto p = q->effectiveSkinHint( Q::Handle ).toPointF();
        const auto s = q->strutSizeHint( Q::Handle );
        const auto w = s.width() / 2;
        const auto h = s.height() / 2;

        if ( p == QPointF{ -1, -1 } )
        {
            return {};
        }

        const auto x = c.x() + p.x() * r;
        const auto y = c.y() + p.y() * r;

        return {
            QPointF{ x - w, y - h },
            QPointF{ x + w, y + h },
        };
    }

    return QskSkinlet::subControlRect( skinnable, contentsRect, subcontrol );
}

QSGNode* QskColorCircleSkinlet::updateSubNode(
    const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* node ) const
{
    const auto* const q = static_cast< const Q* >( skinnable );

    if ( nodeRole == CircleRole )
    {
        const auto gradient = q->gradientHint( Q::Circle );
        auto* const circle = QskSGNode::ensureNode< QskCircleNode >( node );
        circle->update( q->contentsRect(), gradient );
        return circle;
    }

    if ( nodeRole == HandleRole )
    {
        return updateBoxNode( skinnable, node, Q::Handle );
    }

    return QskSkinlet::updateSubNode( skinnable, nodeRole, node );
}

QSizeF QskColorCircleSkinlet::sizeHint(
    const QskSkinnable* skinnable, Qt::SizeHint which, const QSizeF& ) const
{
    if ( which != Qt::PreferredSize )
    {
        return {};
    }

    const auto* const q = static_cast< const Q* >( skinnable );
    const auto rect = q->contentsRect();
    const auto size = qMin( rect.width(), rect.height() );
    return { size, size };
}