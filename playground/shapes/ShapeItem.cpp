/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#include "ShapeItem.h"

#include <QskStrokeNode.h>
#include <QskShapeNode.h>
#include <QskSGNode.h>

ShapeItem::ShapeItem( QQuickItem* parent )
    : QskControl( parent )
{
    setMargins( 20 );
}

ShapeItem::~ShapeItem()
{
}

void ShapeItem::setPen( const QPen& pen )
{
    if ( pen != m_pen )
    {
        m_pen = pen;
        update();
    }
}

QPen ShapeItem::pen() const
{
    return m_pen;
}

void ShapeItem::setGradient( const QColor& c1, const QColor& c2 )
{
    if ( c1 != m_fillColor[0] || c2 != m_fillColor[1] )
    {
        m_fillColor[0] = c1;
        m_fillColor[1] = c2;

        update();
    }
}

void ShapeItem::setPath( const QPainterPath& path )
{
    if ( path != m_path )
    {
        m_path = path;
        update();
    }
}

QPainterPath ShapeItem::path() const
{
    return m_path;
}

void ShapeItem::updateNode( QSGNode* parentNode )
{
    enum NodeRole
    {
        FillRole,
        BorderRole
    };

    const auto rect = contentsRect();

    /*
        The triangulators in the nodes are able to do transformations
        on the fly. TODO ...
     */
    const auto path = scaledPath( rect );

    auto fillNode = static_cast< QskShapeNode* >(
        QskSGNode::findChildNode( parentNode, FillRole ) );

    if ( path.isEmpty() || rect.isEmpty() )
    {
        delete fillNode;
    }
    else
    {
        if ( fillNode == nullptr )
        {
            fillNode = new QskShapeNode;
            QskSGNode::setNodeRole( fillNode, FillRole );
        }

        if ( m_fillColor[0] != m_fillColor[1] )
        {
            QLinearGradient gradient;
            gradient.setStart( rect.topLeft() );
            gradient.setFinalStop( rect.bottomRight() );
            gradient.setColorAt( 0.0, m_fillColor[0] );
            gradient.setColorAt( 1.0, m_fillColor[1] );

            fillNode->updateNode( path, &gradient );
        }
        else
        {
            fillNode->updateNode( path, m_fillColor[0] );
        }

        if ( fillNode->parent() != parentNode )
            parentNode->prependChildNode( fillNode );
    }

    auto borderNode = static_cast< QskStrokeNode* >(
        QskSGNode::findChildNode( parentNode, BorderRole ) );

    if ( path.isEmpty() || rect.isEmpty() )
    {
        delete borderNode;
    }
    else
    {
        if ( borderNode == nullptr )
        {
            borderNode = new QskStrokeNode;
            QskSGNode::setNodeRole( borderNode, BorderRole );
        }

        borderNode->updateNode( path, m_pen );

        if ( borderNode->parent() != parentNode )
            parentNode->appendChildNode( borderNode );
    }
}

QPainterPath ShapeItem::scaledPath( const QRectF& rect ) const
{
    // does not center properly. TODO
    const auto pw = 2 * m_pen.width();

    const auto pathRect = m_path.controlPointRect();
    const auto r = rect.adjusted( pw, pw, -pw, -pw );

    auto transform = QTransform::fromTranslate( r.left(), r.top() );
    transform.scale( r.width() / pathRect.width(), r.height() / pathRect.height() );

    return transform.map( m_path );
}

#include "moc_ShapeItem.cpp"
