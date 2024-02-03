/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 *           SPDX-License-Identifier: BSD-3-Clause
 *****************************************************************************/

#include "DialogPage.h"
#include "QskBox.h"
#include "QskBoxBorderColors.h"
#include "QskControl.h"
#include "QskGradient.h"
#include "QskGridBox.h"
#include "QskSizePolicy.h"
#include "QskSlider.h"
#include "QskTextLabel.h"

#include <QskBoxBorderMetrics.h>
#include <QskBoxShapeMetrics.h>
#include <QskDialog.h>
#include <QskLinearBox.h>
#include <QskPushButton.h>
#include <QskStandardSymbol.h>
#include <QskTabBar.h>
#include <QskTabView.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qquickwindow.h>

namespace
{
    class Button : public QskPushButton
    {
      public:
        Button( const QString& text, QQuickItem* parent = nullptr )
            : QskPushButton( text, parent )
        {
        }
    };

    class ButtonBox : public QskLinearBox
    {
      public:
        ButtonBox( QQuickItem* parent = nullptr )
            : QskLinearBox( Qt::Horizontal, 2, parent )
        {
            setObjectName( "ButtonBox" );

            setMargins( 10 );
            setSpacing( 20 );

            auto messageButton = new Button( "Message", this );
            connect( messageButton, &Button::clicked, this, &ButtonBox::execMessage );

            auto informationButton = new Button( "Information", this );
            connect( informationButton, &Button::clicked, this, &ButtonBox::execInformation );

            auto questionButton = new Button( "Question", this );
            connect( questionButton, &Button::clicked, this, &ButtonBox::execQuestion );

            auto warningButton = new Button( "Warning", this );
            connect( warningButton, &Button::clicked, this, &ButtonBox::execWarning );

            auto criticalButton = new Button( "Critical", this );
            connect( criticalButton, &Button::clicked, this, &ButtonBox::execCritical );

            auto selectButton = new Button( "Selection", this );
            connect( selectButton, &Button::clicked, this, &ButtonBox::execSelection );

            auto colortButton = new Button( "Color", this );
            connect( colortButton, &Button::clicked, this, [ this ]() { execColor( window() ); } );

            setExtraSpacingAt( Qt::BottomEdge );
        }

      private:
        void execMessage()
        {
            qskDialog->message(
                "Message", "Request vector, over.", QskStandardSymbol::NoSymbol, QskDialog::Close );
        }

        void execInformation()
        {
            qskDialog->information( "Information", "We have clearance, Clarence." );
        }

        void execQuestion()
        {
            qskDialog->question( "Question", "Roger, Roger. Do we have a vector, Victor ?" );
        }

        void execWarning()
        {
            qskDialog->warning( "Warning", "We have clearance, Clarence." );
        }

        void execCritical()
        {
            qskDialog->critical( "Critical", "That's Clarence Oveur. Over." );
        }

        void execSelection()
        {
            // of course we all love "The Teens"
            const QStringList entries = { "Give Me More", "Gimme Gimme Your Love",
                "1-2-3-4 Red Light", "New York", "If You Walk Away", "Eloise", "On The Radio",
                "We Are The Teens", "Never Gonna Tell No Lie To You", "Walking On Sunshine ",
                "Get Out Of My Mind", "Cover Girl ", "Here I Stand", "Gypsy Caravan",
                "It´s Good To Have A Friend", "We´ll Have A Party Tonite ´nite", "Automatic World",
                "Gimme Gimme Gimme Gimme Gimme Your Love" };

            qskDialog->select( "Here we go ...", "Hot Hot Hot", entries, 7 );
        }

        void execColor( QQuickWindow* window )
        {
            auto* const layout = new QskLinearBox( Qt::Vertical, window->contentItem() );
            layout->setPanel( true );
            auto* const tabBar = new QskTabView( layout );
            tabBar->addTab( "Sliders", [ window ]() {
                auto* const layout = new QskGridBox( window->contentItem() );
                auto* const color = new QskSlider( Qt::Horizontal );
                auto* const brightness = new QskSlider( Qt::Horizontal );
                auto* const grayscale = new QskSlider( Qt::Horizontal );
                auto* const alpha = new QskSlider( Qt::Horizontal );
                auto* const preview = new QskBox;
                auto* const text = new QskTextLabel( "#00000000" );
                preview->setPanel( true );
                alpha->setValue( 1.0 );
                layout->addItem( preview, 0, 0, 4, 1 );
                layout->addItem( color, 0, 1 );
                layout->addItem( grayscale, 1, 1 );
                layout->addItem( brightness, 2, 1 );
                layout->addItem( alpha, 3, 1 );
                layout->addItem( text, 4, 0 );

                preview->setSizePolicy( Qt::Vertical, QskSizePolicy::ConstrainedMinimum );
                text->setAlignmentHint( QskTextLabel::Text, Qt::AlignHCenter | Qt::AlignVCenter );

                static const QskGradientStops stopsRGB = {
                    { 0.0000, QColor::fromRgb( 255, 0, 0 ) },
                    { 0.1667, QColor::fromRgb( 255, 255, 0 ) },
                    { 0.3333, QColor::fromRgb( 0, 255, 0 ) },
                    { 0.5000, QColor::fromRgb( 0, 255, 255 ) },
                    { 0.6667, QColor::fromRgb( 0, 0, 255 ) },
                    { 0.8333, QColor::fromRgb( 255, 0, 255 ) },
                    { 1.0000, QColor::fromRgb( 255, 0, 0 ) },
                };

                static const QskGradientStops stopsGrayscale = {
                    { 0.0000, Qt::black },
                    { 1.0000, Qt::white },
                };

                QskGradient gradient;
                gradient.setLinearDirection( Qt::Horizontal );
                gradient.setStops( stopsRGB );

                {
                    QskGradient gradient;
                    gradient.setLinearDirection( Qt::Horizontal );
                    gradient.setStops( stopsGrayscale );
                    grayscale->setGradientHint( QskSlider::Groove, gradient );
                }

                color->setGradientHint( QskSlider::Groove, gradient );

                for ( QskSlider* slider : { color, brightness, alpha, grayscale } )
                {
                    slider->setGradientHint( QskSlider::Fill, {} );
                }

                auto colorFrom = []( QskSlider* slider ) {
                    return slider->gradientHint( QskSlider::Groove )
                        .extracted( slider->valueAsRatio(), slider->valueAsRatio() )
                        .startColor();
                };

                auto updateColor = [ = ]( QskSlider* sender ) {
                    const auto ratio = sender->valueAsRatio();
                    const auto rgb = sender->gradientHint( QskSlider::Groove )
                                         .extracted( ratio, ratio )
                                         .startColor();

                    {
                        auto c = colorFrom( color ).rgb();

                        QskGradient gradient;
                        gradient.setLinearDirection( Qt::Horizontal );
                        gradient.setStops( { { 0.0, Qt::white }, { 0.5, c }, { 1.0, Qt::black } } );
                        brightness->setGradientHint( QskSlider::Groove, gradient );
                    }

                    {
                        QskGradient gradient;
                        gradient.setLinearDirection( Qt::Horizontal );
                        gradient.setStops( { { 0.0, Qt::transparent },
                            { 1.0, { rgb.red(), rgb.green(), rgb.blue() } } } );
                        alpha->setGradientHint( QskSlider::Groove, gradient );
                    }

                    {
                        auto color = preview->color( QskBox::Panel );
                        color.setRed( rgb.red() );
                        color.setGreen( rgb.green() );
                        color.setBlue( rgb.blue() );
                        preview->setColor( QskBox::Panel, color );
                        preview->setBoxBorderColorsHint( QskBox::Panel, color.rgb() | 0xff000000 );

                        text->setText( color.name( QColor::HexArgb ) );
                    }
                };

                auto updateAlpha = [ = ]( qreal alpha ) {
                    auto color = preview->color( QskBox::Panel );
                    color.setAlphaF( alpha );
                    preview->setColor( QskBox::Panel, color );
                    text->setText( color.name( QColor::HexArgb ) );
                };

                QObject::connect( color, &QskSlider::valueChanged, preview,
                    [ updateColor, sender = color ]() { updateColor( sender ); } );
                QObject::connect( grayscale, &QskSlider::valueChanged, preview,
                    [ updateColor, sender = grayscale ]() { updateColor( sender ); } );
                QObject::connect( brightness, &QskSlider::valueChanged, preview,
                    [ updateColor, sender = brightness ]() { updateColor( sender ); } );
                QObject::connect( alpha, &QskSlider::valueChanged, preview, updateAlpha );

                preview->setFixedSize( 80, 80 );
                preview->setBoxShapeHint( QskBox::Panel, { 4 } );
                preview->setBoxBorderColorsHint( QskBox::Panel, { Qt::black } );
                preview->setBoxBorderMetricsHint( QskBox::Panel, { 2 } );
                preview->setMarginHint( QskBox::Panel, 2 );

                layout->setColumnStretchFactor(0, 0);
                layout->setColumnStretchFactor(1, 99);

                updateColor( color );
                updateAlpha( alpha->valueAsRatio() );
                return layout;
            }() );
            layout->addSpacer( -1, 99 );
            auto* const button = new QskPushButton( "Close", layout );
            layout->setDefaultAlignment( Qt::AlignRight | Qt::AlignHCenter );
            QObject::connect( button, &QskPushButton::clicked, layout, &QskLinearBox::deleteLater );
        }
    };
}

DialogPage::DialogPage( QQuickItem* parent )
    : Page( Qt::Horizontal, parent )
{
    ( void ) new ButtonBox( this );
}
