#include "QskColorWheelSkinlet.h"
#include "QskColorWheel.h"

#include <QskArcMetrics.h>
#include <qmath.h>

using Q = QskColorWheel;

QskColorWheelSkinlet::QskColorWheelSkinlet( QskSkin* skin )
    : QskSkinlet( skin )
{
    setNodeRoles( { ArcRole, HandleRole } );
}

QRectF QskColorWheelSkinlet::subControlRect( const QskSkinnable* skinnable,
    const QRectF& contentsRect, QskAspect::Subcontrol subcontrol ) const
{
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

    const auto* const q = static_cast< const Q* >( skinnable );
    const auto rect = q->contentsRect();
    const auto size = qMin( rect.width(), rect.height() );
    return { size, size };
}