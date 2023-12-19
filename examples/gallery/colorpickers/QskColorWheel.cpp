#include "QskColorWheel.h"
#include <QskArcMetrics.h>
#include <QskBoxBorderColors.h>
#include <QskBoxBorderMetrics.h>
#include <QskBoxShapeMetrics.h>
#include <QskFunctions.h>
#include <QskGradient.h>
#include <qmath.h>

namespace
{
    QColor qskColorAt( const QskGradient& gradient, qreal ratio )
    {
        return gradient.extracted( ratio, ratio ).startColor();
    }

    QColor qskColorForAngle( const QskGradient& gradient, qreal angle )
    {
        angle = ( angle < 0.0 ? angle + 2 * M_PI : angle );
        const auto ratio = qAbs( angle / ( 2 * M_PI ) );
        return qskColorAt( gradient, ratio );
    }
}

QSK_SUBCONTROL( QskColorWheel, Arc )
QSK_SUBCONTROL( QskColorWheel, Handle )

QskColorWheel::QskColorWheel( QQuickItem* parent )
    : QskControl( parent )
{
    setAcceptedMouseButtons( Qt::LeftButton );

    static const QskGradientStops stops = {
        { 0.0000, QColor::fromRgb( 255, 0, 0 ) },
        { 0.1667, QColor::fromRgb( 255, 255, 0 ) },
        { 0.3333, QColor::fromRgb( 0, 255, 0 ) },
        { 0.5000, QColor::fromRgb( 0, 255, 255 ) },
        { 0.6667, QColor::fromRgb( 0, 0, 255 ) },
        { 0.8333, QColor::fromRgb( 255, 0, 255 ) },
        { 1.0000, QColor::fromRgb( 255, 0, 0 ) },
    };

    QskGradient gradient( stops );
    setGradientHint( Arc, gradient );
    setArcMetricsHint( Arc, { 0, 360, 16 } );

    setColor( Handle, Qt::white );
    setBoxBorderColorsHint( Handle, { Qt::white } );
    setBoxBorderMetricsHint( Handle, { 2 } );
    setBoxShapeHint( Handle, { 100.0, 100.0, 100.0, 100.0, Qt::RelativeSize } );
}

QColor QskColorWheel::selectedColor() const
{
    return m_selectedColor;
}

void QskColorWheel::setSelectedColor( const QColor& color )
{
    if ( m_selectedColor != color )
    {
        m_selectedColor = color;
        setColor( Handle, color );
        Q_EMIT selectedColorChanged( m_selectedColor );
    }
}

void QskColorWheel::mousePressEvent( QMouseEvent* event )
{
}

void QskColorWheel::mouseReleaseEvent( QMouseEvent* event )
{
}

void QskColorWheel::mouseMoveEvent( QMouseEvent* event )
{
    const auto r = contentsRect();
    const auto ux = qskMapValueRange( event->pos().x(), r.left(), r.right(), -1, +1 );
    const auto uy = qskMapValueRange( event->pos().y(), r.top(), r.bottom(), +1, -1 );

    if ( QVector2D( ux, uy ).length() > 1.0 ) // TODO && Pressed
    {
        event->ignore();
        return;
    }

    const auto rad = qAtan2( uy, ux );
    const auto color = qskColorForAngle( gradientHint( Arc ), rad );
    setPositionHint( Handle, -rad );
    setSelectedColor( color );
}

#include "moc_QskColorWheel.cpp"