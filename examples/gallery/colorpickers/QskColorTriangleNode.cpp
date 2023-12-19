#include "QskColorTriangleNode.h"
#include <qmath.h>

namespace
{
    static constexpr auto vertexCount = 3;
}

QskColorTriangleNode::QskColorTriangleNode()
{
    auto* const geometry =
        new QSGGeometry( QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount );
    geometry->setDrawingMode( QSGGeometry::DrawTriangles );
    setGeometry( geometry );

    auto* const material = new QSGVertexColorMaterial;
    setMaterial( material );
}

void QskColorTriangleNode::update(
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