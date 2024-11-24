/******************************************************************************
 * QSkinny - Copyright (C) The authors
 *           SPDX-License-Identifier: BSD-3-Clause
 *****************************************************************************/

#include "QskBoxNode.h"
#include "QskBoxShadowNode.h"
#include "QskBoxRectangleNode.h"
#include "QskSGNode.h"

#include "QskGradient.h"
#include "QskGradientDirection.h"
#include "QskShadowMetrics.h"
#include "QskBoxBorderMetrics.h"
#include "QskBoxBorderColors.h"
#include "QskBoxShapeMetrics.h"
#include "QskRgbValue.h"

namespace QskBoxNodeImpl
{
    enum Role
    {
        ShadowRole,
        ShadowFillRole,
        BoxRole,
        FillRole
    };

    static void qskUpdateChildren( QSGNode* parentNode, quint8 role, QSGNode* node )
    {
        static const QVector< quint8 > roles =
            { QskBoxNodeImpl::ShadowRole, QskBoxNodeImpl::ShadowFillRole, QskBoxNodeImpl::BoxRole, QskBoxNodeImpl::BoxRole };

        auto oldNode = QskSGNode::findChildNode( parentNode, role );
        QskSGNode::replaceChildNode( roles, role, parentNode, oldNode, node );
    }

    template< typename Node >
    static inline Node* qskNode( QSGNode* parentNode, quint8 role )
    {
        using namespace QskSGNode;

        auto node = static_cast< Node* > ( findChildNode( parentNode, role ) );

        if ( node == nullptr )
        {
            node = new Node();
            setNodeRole( node, role );
        }

        return node;
    }
}

QskBoxNode::QskBoxNode()
{
}

QskBoxNode::~QskBoxNode()
{
}

void QskBoxNode::updateNode( const QQuickWindow* window, const QRectF& rect,
    const QskBoxShapeMetrics& shapeMetrics, const QskBoxBorderMetrics& borderMetrics,
    const QskBoxBorderColors& borderColors, const QskGradient& gradient,
    const QskShadowMetrics& shadowMetrics, const QColor& shadowColor )
{
    using namespace QskSGNode;

    QskBoxShadowNode* shadowNode = nullptr;
    QskBoxRectangleNode* shadowFillNode = nullptr;
    QskBoxRectangleNode* rectNode = nullptr;
    QskBoxRectangleNode* fillNode = nullptr;

    if ( !rect.isEmpty() )
    {
        const auto hasFilling = gradient.isVisible();
        const auto hasBorder = !borderMetrics.isNull() && borderColors.isVisible();
        const auto hasShadow = !shadowMetrics.isNull() && QskRgb::isVisible( shadowColor );

        if ( hasShadow )
        {
            const auto shadow = shadowMetrics.toAbsolute( rect.size() );
            const auto shadowRect = shadow.shadowRect( rect );

            auto shadowShape = shapeMetrics;

            switch( static_cast< int >( shadow.shapeMode() ) )
            {
                case QskShadowMetrics::Ellipse:
                    shadowShape.setRadius( 100.0, Qt::RelativeSize );
                    break;

                case QskShadowMetrics::Rectangle:
                    shadowShape.setRadius( 0.0, Qt::AbsoluteSize );
                    break;
            }

            if ( shadow.blurRadius() <= 0.0 )
            {
                // QskBoxRectangleNode allows scene graph batching
                shadowFillNode = QskBoxNodeImpl::qskNode< QskBoxRectangleNode >( this, QskBoxNodeImpl::ShadowFillRole );
                shadowFillNode->updateFilling( window,
                    shadowRect, shadowShape, shadowColor );
            }
            else
            {
                shadowNode = QskBoxNodeImpl::qskNode< QskBoxShadowNode >( this, QskBoxNodeImpl::ShadowRole );
                shadowNode->setShadowData( shadowRect,
                    shadowShape, shadow.blurRadius(), shadowColor );
            }
        }

        if ( hasBorder || hasFilling )
        {
            rectNode = QskBoxNodeImpl::qskNode< QskBoxRectangleNode >( this, QskBoxNodeImpl::BoxRole );

            if ( hasBorder && hasFilling )
            {
                const bool doCombine = rectNode->hasHint( QskFillNode::PreferColoredGeometry )
                    && QskBoxRectangleNode::isCombinedGeometrySupported( gradient );

                if ( !doCombine )
                    fillNode = QskBoxNodeImpl::qskNode< QskBoxRectangleNode >( this, QskBoxNodeImpl::BoxRole );
            }

            if ( fillNode )
            {
                rectNode->updateBorder( window, rect,
                    shapeMetrics, borderMetrics, borderColors );

                fillNode->updateFilling( window, rect,
                    shapeMetrics, borderMetrics, gradient );
            }
            else
            {
                rectNode->updateBox( window, rect,
                    shapeMetrics, borderMetrics, borderColors, gradient );
            }
        }
    }

    QskBoxNodeImpl::qskUpdateChildren( this, QskBoxNodeImpl::ShadowRole, shadowNode );
    QskBoxNodeImpl::qskUpdateChildren( this, QskBoxNodeImpl::ShadowFillRole, shadowFillNode );
    QskBoxNodeImpl::qskUpdateChildren( this, QskBoxNodeImpl::BoxRole, rectNode );
    QskBoxNodeImpl::qskUpdateChildren( this, QskBoxNodeImpl::BoxRole, fillNode );
}
