#ifndef QSK_CIRCLE_NODE_H_
#define QSK_CIRCLE_NODE_H_

#include "QskGradient.h"
#include <QSGGeometryNode>

class QskCircleNode : public QSGGeometryNode
{
  public:
    QskCircleNode();
    void update( const QRectF& rect, const QskGradient& gradient, int vertexCount = 360 );
};

#endif