#include "QskCircleNode.h"
#include "colorpickers/QskCircleNode.h"

#include <QSGGeometry>
#include <QSGVertexColorMaterial>

#include <qmath.h>
#include <qnamespace.h>
#include <qsgnode.h>

#include <QskGradient.h>

QskCircleNode::QskCircleNode()
{
    auto* const geometry = new QSGGeometry( QSGGeometry::defaultAttributes_ColoredPoint2D(), 0 );
    geometry->setDrawingMode( QSGGeometry::DrawTriangleFan );
    setGeometry( geometry );

    auto* const material = new QSGVertexColorMaterial;
    setMaterial( material );
}

void QskCircleNode::update( const QRectF& rect, const QskGradient& gradient, int vertexCount )
{
    constexpr auto additional = 2; // +1 center, +1 for second 0 degree closing
    vertexCount = qMax( 3, vertexCount ) + additional;

    if ( geometry()->vertexCount() != vertexCount )
    {
        geometry()->allocate( vertexCount );
    }

    const auto step = 2 * M_PI / ( vertexCount - additional );
    const auto radius = qMin( rect.width(), rect.height() ) / 2;

    auto* const vertices = geometry()->vertexDataAsColoredPoint2D();
    vertices[ 0 ].set( rect.center().x(), rect.center().y(), 255, 255, 255, 255 );
    for ( int i = 1; i <= vertexCount; i++ )
    {
        const auto angle = ( i - 1 ) * step;
        const auto x = qFastCos( angle ) * radius + rect.center().x();
        const auto y = qFastSin( angle ) * radius + rect.center().y();
        const auto color =
            gradient.extracted( angle / ( 2 * M_PI ), angle / ( 2 * M_PI ) ).startColor();
        vertices[ i ].set( x, y, color.red(), color.green(), color.blue(), color.alpha() );
    }

    geometry()->markVertexDataDirty();
    markDirty( QSGNode::DirtyGeometry );
    markDirty( QSGNode::DirtyMaterial );
}