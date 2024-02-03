#include "QskColorCircle.h"
#include "QskAspect.h"
#include "QskControl.h"
#include "QskGradient.h"
#include "colorpickers/QskColorCircle.h"

#include <QskBoxBorderColors.h>
#include <QskBoxBorderMetrics.h>
#include <QskBoxShapeMetrics.h>
#include <QskFunctions.h>

#include <qmath.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qpoint.h>

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

QSK_SUBCONTROL( QskColorCircle, Circle )
QSK_SUBCONTROL( QskColorCircle, Handle )

QskColorCircle::QskColorCircle( QQuickItem* parent )
    : QskControl( parent )
{
    const QskGradient gradient( {
        { 0.0000, QColor::fromRgb( 255, 0, 0 ) },
        { 0.1667, QColor::fromRgb( 255, 255, 0 ) },
        { 0.3333, QColor::fromRgb( 0, 255, 0 ) },
        { 0.5000, QColor::fromRgb( 0, 255, 255 ) },
        { 0.6667, QColor::fromRgb( 0, 0, 255 ) },
        { 0.8333, QColor::fromRgb( 255, 0, 255 ) },
        { 1.0000, QColor::fromRgb( 255, 0, 0 ) },
    } );

    setGradientHint( Circle, gradient );

    setBoxBorderColorsHint( Handle, Qt::white );
    setBoxBorderMetricsHint( Handle, 2 );
    setBoxShapeHint( Handle, { 100, 100, 100, 100, Qt::AbsoluteSize } );
    setStrutSizeHint( Handle, { 20, 20 } );

    setAcceptedMouseButtons( Qt::LeftButton );
}

void QskColorCircle::mousePressEvent( QMouseEvent* event )
{
}

void QskColorCircle::mouseReleaseEvent( QMouseEvent* event )
{
}

void QskColorCircle::mouseMoveEvent( QMouseEvent* event )
{
    const auto r = subControlRect( Circle );
    const auto ux = qskMapValueRange( event->pos().x(), r.left(), r.right(), -1, +1 );
    const auto uy = qskMapValueRange( event->pos().y(), r.top(), r.bottom(), -1, +1 );

    const auto radius = QVector2D( ux, uy ).length();
    if ( radius > 1.0 )
    {
        event->ignore();
        return;
    }

    const auto rad = qAtan2( uy, ux );
    const auto ratio = rad / ( 2 * M_PI );
    const auto color = qskColorForAngle( gradientHint( Circle ), rad );
    const QskGradient gradient(Qt::white, color);

    const auto effectiveColor = qskColorAt(gradient, radius);

    setColor( Handle, effectiveColor );
    setSkinHint( Handle, QPointF{ ux, uy } );
    setSelectedColor( effectiveColor );
}

QColor QskColorCircle::selectedColor() const
{
    return m_selectedColor;
}

void QskColorCircle::setSelectedColor( const QColor& color )
{
    if ( m_selectedColor != color )
    {
        m_selectedColor = color;
        Q_EMIT selectedColorChanged( m_selectedColor );
    }
}