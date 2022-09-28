/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 * This file may be used under the terms of the QSkinny License, Version 1.0
 *****************************************************************************/

#ifndef QSK_GRADIENT_MATERIAL
#define QSK_GRADIENT_MATERIAL

#include "QskGlobal.h"
#include <qbrush.h>
#include <qsgmaterial.h>

class QSK_EXPORT QskGradientMaterial : public QSGMaterial
{
  public:
    static QskGradientMaterial* create( const QGradient* );

    QGradient::Type gradientType() const;

    const QGradientStops& stops() const;
    QGradient::Spread spread() const;

  protected:
    QskGradientMaterial( QGradient::Type,
        const QGradientStops&, QGradient::Spread );

  private:
    const QGradient::Type m_gradientType;
    const QGradientStops m_stops;
    const QGradient::Spread m_spread;
};

inline QskGradientMaterial::QskGradientMaterial( QGradient::Type type,
        const QGradientStops& stops, QGradient::Spread spread )
    : m_gradientType( type )
    , m_stops( stops )
    , m_spread( spread )
{
}

inline QGradient::Type QskGradientMaterial::gradientType() const
{
    return m_gradientType;
}

inline const QGradientStops& QskGradientMaterial::stops() const
{
    return m_stops;
}

inline QGradient::Spread QskGradientMaterial::spread() const
{
    return m_spread;
}

#endif