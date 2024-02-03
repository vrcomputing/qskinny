#include "QskColorTriangle.h"

#include <QskBoxBorderColors.h>
#include <QskBoxBorderMetrics.h>
#include <QskBoxShapeMetrics.h>
#include <QskFunctions.h>

#include <qmath.h>
#include <qvector3d.h>

namespace
{
    QVector3D cartesian2barycentric(
        const QPointF& p, const QPointF& t1, const QPointF& t2, const QPointF& t3 )
    {
        const auto x = p.x();
        const auto y = p.y();

        const auto x1 = t1.x();
        const auto y1 = t1.y();
        const auto x2 = t2.x();
        const auto y2 = t2.y();
        const auto x3 = t3.x();
        const auto y3 = t3.y();

        const auto y2y3 = y2 - y3, x3x2 = x3 - x2, x1x3 = x1 - x3, y1y3 = y1 - y3, y3y1 = y3 - y1,
                   xx3 = x - x3, yy3 = y - y3;

        const auto d = y2y3 * x1x3 + x3x2 * y1y3, lambda1 = ( y2y3 * xx3 + x3x2 * yy3 ) / d,
                   lambda2 = ( y3y1 * xx3 + x1x3 * yy3 ) / d;

        return { ( float ) lambda1, ( float ) lambda2, ( float ) ( 1.0 - lambda1 - lambda2 ) };
    }

    QColor qskColorInTriangle( const QPointF& p, const QPointF& t1, const QPointF& t2,
        const QPointF& t3, const QColor& c1, const QColor& c2, const QColor& c3 )
    {
        const auto t = cartesian2barycentric( p, t1, t2, t3 );
        const auto r =
            std::floor( ( c1.redF() * t[ 0 ] + c2.redF() * t[ 1 ] + c3.redF() * t[ 2 ] ) * 255.0 );
        const auto g = std::floor(
            ( c1.greenF() * t[ 0 ] + c2.greenF() * t[ 1 ] + c3.greenF() * t[ 2 ] ) * 255.0 );
        const auto b = std::floor(
            ( c1.blueF() * t[ 0 ] + c2.blueF() * t[ 1 ] + c3.blueF() * t[ 2 ] ) * 255.0 );

        if ( t[ 0 ] < 0.0 || t[ 1 ] < 0.0 || t[ 2 ] < 0.0 )
        {
            return {};
        }

        return { int( r ), int( g ), int( b ) };
    }
}

QSK_SUBCONTROL( QskColorTriangle, Triangle )
QSK_SUBCONTROL( QskColorTriangle, Handle )

QskColorTriangle::QskColorTriangle( QQuickItem* parent )
    : QskControl( parent )
{
    setAcceptedMouseButtons( Qt::LeftButton );

    setStrutSizeHint( Handle, { 20, 20 } );
    setColor( Handle, Qt::white );
    setBoxBorderColorsHint( Handle, { Qt::white } );
    setBoxBorderMetricsHint( Handle, { 2 } );
    setBoxShapeHint( Handle, { 100.0, 100.0, 100.0, 100.0, Qt::RelativeSize } );
    setSkinHint( Handle, QPointF{} );
}

QColor QskColorTriangle::selectedColor() const
{
    return m_selectedColor;
}

void QskColorTriangle::setSelectedColor( const QColor& color )
{
    if ( m_selectedColor != color )
    {
        m_selectedColor = color;
        setColor( Handle, color );
        Q_EMIT selectedColorChanged( m_selectedColor );
    }
}

void QskColorTriangle::setColors( const QColor& c1, const QColor& c2, const QColor& c3 )
{
    m_c1 = c1;
    m_c2 = c2;
    m_c3 = c3;
    update();
}

QColor QskColorTriangle::color1() const
{
    return m_c1;
}

QColor QskColorTriangle::color2() const
{
    return m_c2;
}

QColor QskColorTriangle::color3() const
{
    return m_c3;
}

void QskColorTriangle::mousePressEvent( QMouseEvent* event )
{
}

void QskColorTriangle::mouseReleaseEvent( QMouseEvent* event )
{
}

void QskColorTriangle::mouseMoveEvent( QMouseEvent* event )
{
    const auto rect = subControlRect( Triangle );
    if ( rect.contains( event->pos() ) )
    {
        static constexpr auto step = 2 * M_PI / 3;
        const auto angle = positionHint( Triangle );
        const auto radius = qMin( rect.width(), rect.height() ) / 2;
        QTransform transformation;
        transformation.rotate( qRadiansToDegrees( angle ), Qt::ZAxis );

        // TODO affine

        auto t1 = QPointF{ qFastCos( 0 * step ), qFastSin( 0 * step ) };
        auto t2 = QPointF{ qFastCos( 1 * step ), qFastSin( 1 * step ) };
        auto t3 = QPointF{ qFastCos( 2 * step ), qFastSin( 2 * step ) };

        t1 = transformation.map( t1 );
        t2 = transformation.map( t2 );
        t3 = transformation.map( t3 );

        t1 *= radius;
        t2 *= radius;
        t3 *= radius;

        t1 += rect.center();
        t2 += rect.center();
        t3 += rect.center();

        const auto color = qskColorInTriangle( event->pos(), t1, t2, t3, m_c1, m_c2, m_c3 );
        if ( color.isValid() )
        {
            setSelectedColor( color );

            const auto x = qskMapValueRange( event->pos().x(), rect.left(), rect.right(), -1, +1 );
            const auto y = qskMapValueRange( event->pos().y(), rect.top(), rect.bottom(), +1, -1 );
            setSkinHint( Handle, QPointF{ x, y } );
        }
    }
    event->ignore();
}

#include "moc_QskColorTriangle.cpp"