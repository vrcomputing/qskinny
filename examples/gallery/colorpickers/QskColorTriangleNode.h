#ifndef QSK_COLOR_TRIANGLE_NODE_H_
#define QSK_COLOR_TRIANGLE_NODE_H_

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>

class QskColorTriangleNode : public QSGGeometryNode
{
    using rad = qreal;

  public:
    QskColorTriangleNode();

    void update(
        const QRectF& rect, const QColor& c1, const QColor& c2, const QColor& c3, rad angle );
};

#endif