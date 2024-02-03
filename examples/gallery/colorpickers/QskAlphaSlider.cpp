#include "QskAlphaSlider.h"

QSK_SUBCONTROL( QskAlphaSlider, Grid )

QskAlphaSlider::QskAlphaSlider( QQuickItem* const parent )
    : QskSlider( parent )
{
    setGradientHint( Grid, { "#B2B2B2", "#EEEEEE" } );
    setGradientHint( Fill, {});
    setMetric( Groove | QskAspect::Size, 16 );
    setMetric( Fill | QskAspect::Size, 16 );
    setMetric( Grid | QskAspect::Size, 8 );
}