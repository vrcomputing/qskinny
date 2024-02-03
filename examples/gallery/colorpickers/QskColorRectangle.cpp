#include "QskColorRectangle.h"

#include <QskBoxBorderColors.h>
#include <QskBoxBorderMetrics.h>
#include <QskBoxShapeMetrics.h>

#include <qvector3d.h>

QSK_SUBCONTROL( QskColorRectangle, Panel )
QSK_SUBCONTROL( QskColorRectangle, Handle )

namespace
{
    double qskMapValueRange(
        double value, double srcMin, double srcMax, double dstMin, double dstMax )
    {
        value = std::min( std::max( value, srcMin ), srcMax );
        double percentage = ( value - srcMin ) / ( srcMax - srcMin );
        double mappedValue = dstMin + percentage * ( dstMax - dstMin );
        return mappedValue;
    }

    QColor qskInterpolatedColor( const QColor& topLeft, const QColor& topRight,
        const QColor& bottomLeft, const QColor& bottomRight, const QPointF& position )
    {
        const auto x = position.x();
        const auto y = position.y();

        const QColor topInterpolated( int( ( 1.0 - x ) * topLeft.red() + x * topRight.red() ),
            int( ( 1.0 - x ) * topLeft.green() + x * topRight.green() ),
            int( ( 1.0 - x ) * topLeft.blue() + x * topRight.blue() ) );

        const QColor bottomInterpolated(
            int( ( 1.0 - x ) * bottomLeft.red() + x * bottomRight.red() ),
            int( ( 1.0 - x ) * bottomLeft.green() + x * bottomRight.green() ),
            int( ( 1.0 - x ) * bottomLeft.blue() + x * bottomRight.blue() ) );

        return { int( ( 1.0 - y ) * topInterpolated.red() + y * bottomInterpolated.red() ),
            int( ( 1.0 - y ) * topInterpolated.green() + y * bottomInterpolated.green() ),
            int( ( 1.0 - y ) * topInterpolated.blue() + y * bottomInterpolated.blue() ) };
    }
}

QskColorRectangle::QskColorRectangle( QQuickItem* parent )
    : QskControl( parent )
{
    setAcceptedMouseButtons( Qt::LeftButton );

    setBoxBorderColorsHint( Handle, Qt::white );
    setBoxBorderMetricsHint( Handle, 2 );
    setBoxShapeHint( Handle, { 100, 100, 100, 100, Qt::AbsoluteSize } );
    setStrutSizeHint( Handle, { 20, 20 } );
    setSkinHint( Handle, QPointF{ 50, 50 } );
}

QColor QskColorRectangle::selectedColor() const
{
    return m_selectedColor;
}

void QskColorRectangle::setCornerColor( const Qt::Corner corner, const QColor& color )
{
    switch ( corner )
    {
        case Qt::TopLeftCorner:
        case Qt::TopRightCorner:
        case Qt::BottomLeftCorner:
        case Qt::BottomRightCorner:
            m_cornerColor[ corner ] = color;
            update();
            break;
        default:
            break;
    }
}

QColor QskColorRectangle::cornerColor( const Qt::Corner corner ) const
{
    switch ( corner )
    {
        case Qt::TopLeftCorner:
        case Qt::TopRightCorner:
        case Qt::BottomLeftCorner:
        case Qt::BottomRightCorner:
            return m_cornerColor[ corner ];
        default:
            return {};
    }
}

void QskColorRectangle::mousePressEvent( QMouseEvent* event )
{
}

void QskColorRectangle::mouseReleaseEvent( QMouseEvent* event )
{
}

void QskColorRectangle::mouseMoveEvent( QMouseEvent* event )
{
    const auto r = subControlRect( Panel );
    if ( r.contains( event->pos() ) )
    {
        const auto ux = qskMapValueRange( event->pos().x(), r.left(), r.right(), 0, 1 );
        const auto uy = qskMapValueRange( event->pos().y(), r.top(), r.bottom(), 0, 1 );
        const auto uv = QPointF{ ux, uy };
        
        const auto color = qskInterpolatedColor( m_cornerColor[ Qt::TopLeftCorner ],
            m_cornerColor[ Qt::TopRightCorner ], m_cornerColor[ Qt::BottomLeftCorner ],
            m_cornerColor[ Qt::BottomRightCorner ], uv );

        setColor( Handle, color );
        setSkinHint( Handle, uv );
        setSelectedColor( color );
    }
}

void QskColorRectangle::setSelectedColor( const QColor& color )
{
    if ( m_selectedColor != color )
    {
        m_selectedColor = color;
        update();
        Q_EMIT selectedColorChanged( m_selectedColor );
    }
}

#include "moc_QskColorRectangle.cpp"