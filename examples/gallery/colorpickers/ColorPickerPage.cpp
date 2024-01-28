#include "ColorPickerPage.h"

#include "QskColorCircle.h"
#include "QskColorCircleSkinlet.h"
#include "QskColorRectangle.h"
#include "QskColorRectangleSkinlet.h"
#include "QskColorTriangle.h"
#include "QskColorTriangleSkinlet.h"
#include "QskColorWheel.h"
#include "QskColorWheelSkinlet.h"

#include "colorpickers/QskAlphaSlider.h"
#include "colorpickers/QskAlphaSliderSkinlet.h"
#include "colorpickers/QskColorCircle.h"
#include "colorpickers/QskColorCircleSkinlet.h"
#include "colorpickers/QskColorRectangle.h"
#include "colorpickers/QskColorTriangle.h"


#include <QskArcMetrics.h>
#include <QskGridBox.h>
#include <QskSlider.h>
#include <QskTextLabel.h>

#include <qmath.h>

template< typename Skinnable, typename Skinlet, class... Types >
Skinnable* create( Types&&... args, QQuickItem* parent = nullptr )
{
    auto* const control = new Skinnable( std::forward< Types >( args )..., parent );
    auto* const skinlet = new Skinlet;
    control->setSkinlet( skinlet );
    skinlet->setOwnedBySkinnable( true );
    return control;
}

ColorPickerPage::ColorPickerPage( QQuickItem* parent )
    : Page( parent )
{
    auto* const layout = new QskGridBox( this );

    auto* const colorWheel = create< QskColorWheel, QskColorWheelSkinlet >( layout );
    auto* const colorWheel2 = create< QskColorWheel, QskColorWheelSkinlet >( layout );
    auto* const colorWheelSlider = new QskSlider( layout );

    auto* const colorTriangle = create< QskColorTriangle, QskColorTriangleSkinlet >( layout );
    auto* const colorTriangle2 = create< QskColorTriangle, QskColorTriangleSkinlet >( layout );
    auto* const colorTriangleSlider = new QskSlider( layout );

    auto* const colorRectangle = create< QskColorRectangle, QskColorRectangleSkinlet >( layout );
    auto* const colorRectangle2 = create< QskColorRectangle, QskColorRectangleSkinlet >( layout );

    auto* const colorCircle = create< QskColorCircle, QskColorCircleSkinlet >( layout );
    auto* const colorCircle2 = create< QskColorCircle, QskColorCircleSkinlet >( layout );

    auto* const alphaSlider = create< QskAlphaSlider, QskAlphaSliderSkinlet >( layout );

    colorWheel2->setGradientHint( QskColorWheel::Arc,
        { { { 0.0, Qt::red }, { 0.3333, Qt::green }, { 0.6666, Qt::blue }, { 1.0, Qt::red } } } );
    colorTriangle2->setColors( Qt::red, Qt::green, Qt::blue );
    colorRectangle2->setCornerColor( Qt::TopLeftCorner, Qt::red );
    colorRectangle2->setCornerColor( Qt::TopRightCorner, Qt::green );
    colorRectangle2->setCornerColor( Qt::BottomLeftCorner, Qt::blue );
    colorRectangle2->setCornerColor( Qt::BottomRightCorner, Qt::yellow );

    colorCircle2->setGradientHint( QskColorCircle::Circle,
        { { { 0.0, Qt::red }, { 0.3333, Qt::green }, { 0.6666, Qt::blue }, { 1.0, Qt::red } } } );

    layout->addItem( new QskTextLabel( "QskColorWheel" ), 0, 0 );
    layout->addItem( colorWheel, 1, 0 );
    layout->addItem( colorWheel2, 2, 0 );
    layout->addItem( colorWheelSlider, 3, 0 );
    layout->addItem( new QskTextLabel( "QskColorTriangle" ), 0, 1 );
    layout->addItem( colorTriangle, 1, 1 );
    layout->addItem( colorTriangle2, 2, 1 );
    layout->addItem( colorTriangleSlider, 3, 1 );
    layout->addItem( new QskTextLabel( "QskColorRectangle" ), 0, 2 );
    layout->addItem( colorRectangle, 1, 2 );
    layout->addItem( colorRectangle2, 2, 2 );
    layout->addItem( new QskTextLabel( "QskColorCircle" ), 0, 3 );
    layout->addItem( colorCircle, 1, 3 );
    layout->addItem( colorCircle2, 2, 3 );
    layout->addItem( new QskTextLabel( "QskAlphaSlider" ), 0, 4 );
    layout->addItem( alphaSlider, 1, 4 );

    for ( int col = 0; col < layout->columnCount(); col++ )
    {
        layout->setColumnStretchFactor( col, 1 );
    }

    connect( colorWheel, &QskColorWheel::selectedColorChanged, colorTriangle,
        [ colorTriangle ](
            const QColor& color ) { colorTriangle->setColors( color, Qt::black, Qt::white ); } );

    connect( colorCircle, &QskColorCircle::selectedColorChanged, colorTriangle,
        [ colorTriangle ](
            const QColor& color ) { colorTriangle->setColors( color, Qt::black, Qt::white ); } );

    connect( colorWheelSlider, &QskSlider::valueChanged, this, [ q = colorWheel ]( qreal v ) {
        auto metrics = q->arcMetricsHint( QskColorWheel::Arc );
        metrics.setThickness( v * qMin( q->width(), q->height() ) / 2 );
        q->setArcMetricsHint( QskColorWheel::Arc, metrics );
    } );

    connect( colorTriangleSlider, &QskSlider::valueChanged, this, [ = ]( qreal v ) {
        colorTriangle->setPositionHint( QskColorTriangle::Triangle, 2 * M_PI * v );
    } );

    connect( colorWheel, &QskColorWheel::selectedColorChanged, colorRectangle,
        [ = ]( const QColor& color ) {
            colorRectangle->setCornerColor( Qt::TopLeftCorner, Qt::white );
            colorRectangle->setCornerColor( Qt::TopRightCorner, color );
            colorRectangle->setCornerColor( Qt::BottomLeftCorner, Qt::black );
            colorRectangle->setCornerColor( Qt::BottomRightCorner, Qt::black );
        } );

    connect( colorCircle, &QskColorCircle::selectedColorChanged, colorRectangle,
        [ = ]( const QColor& color ) {
            colorRectangle->setCornerColor( Qt::TopLeftCorner, Qt::white );
            colorRectangle->setCornerColor( Qt::TopRightCorner, color );
            colorRectangle->setCornerColor( Qt::BottomLeftCorner, Qt::black );
            colorRectangle->setCornerColor( Qt::BottomRightCorner, Qt::black );
        } );

    connect( colorCircle, &QskColorCircle::selectedColorChanged, alphaSlider,
        [ = ]( const QColor& color ) {
            QskGradient gradient{color, Qt::transparent};
            gradient.setLinearDirection(alphaSlider->orientation());
            alphaSlider->setGradientHint(QskAlphaSlider::Groove, gradient);
        } );
}