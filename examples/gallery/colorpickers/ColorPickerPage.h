/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 *           SPDX-License-Identifier: BSD-3-Clause
 *****************************************************************************/

#pragma once

#include "Page.h"
#include "QskAspect.h"
#include "QskSkin.h"

#include <QskControl.h>
#include <QskSkinlet.h>
#include <qcolor.h>
#include <qnamespace.h>
#include <qobjectdefs.h>

class ColorPickerPage : public Page
{
  public:
    explicit ColorPickerPage( QQuickItem* = nullptr );
};

class QskColorWheel : public QskControl
{
    Q_OBJECT
    Q_PROPERTY(
        QColor selectedColor READ selectedColor WRITE setSelectedColor NOTIFY selectedColorChanged )
  public:
    QSK_SUBCONTROLS( Arc, Handle )
    explicit QskColorWheel( QQuickItem* parent = nullptr );

    QColor selectedColor() const;
    void setSelectedColor( const QColor& color );

  protected:
    void mousePressEvent( QMouseEvent* event ) override;
    void mouseReleaseEvent( QMouseEvent* event ) override;
    void mouseMoveEvent( QMouseEvent* event ) override;

  Q_SIGNALS:
    void selectedColorChanged( const QColor& color );

  private:
    QColor m_selectedColor;
};

class QskColorWheelSkinlet : public QskSkinlet
{
  public:
    enum NodeRoles
    {
        ArcRole,
        HandleRole
    };

    QskColorWheelSkinlet( QskSkin* skin = nullptr );

    QRectF subControlRect(
        const QskSkinnable* skinnable, const QRectF&, QskAspect::Subcontrol ) const override;

    QSGNode* updateSubNode(
        const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* ) const override;

    QSizeF sizeHint(
        const QskSkinnable* skinnable, Qt::SizeHint which, const QSizeF& ) const override;
};

class QskColorTriangle : public QskControl
{
    Q_OBJECT
    Q_PROPERTY(
        QColor selectedColor READ selectedColor WRITE setSelectedColor NOTIFY selectedColorChanged )

  public:
    QSK_SUBCONTROLS( Triangle, Handle )
    explicit QskColorTriangle( QQuickItem* parent = nullptr );

    QColor selectedColor() const;

    void setColors(const QColor& c1, const QColor& c2, const QColor& c3);
    QColor color1()const;
    QColor color2()const;
    QColor color3()const;

  Q_SIGNALS:
    void selectedColorChanged( const QColor& color );

  protected:
    void mousePressEvent( QMouseEvent* event ) override;
    void mouseReleaseEvent( QMouseEvent* event ) override;
    void mouseMoveEvent( QMouseEvent* event ) override;

  private:
    void setSelectedColor( const QColor& color );

    QColor m_selectedColor;
    QColor m_c1;
    QColor m_c2 = Qt::black;
    QColor m_c3 = Qt::white;
};

class QskColorTriangleSkinlet : public QskSkinlet
{
  public:
    enum NodeRoles
    {
        TriangleRole,
        HandleRole
    };

    QskColorTriangleSkinlet( QskSkin* skin = nullptr );

    QRectF subControlRect(
        const QskSkinnable* skinnable, const QRectF&, QskAspect::Subcontrol ) const override;

    QSGNode* updateSubNode(
        const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* ) const override;

    QSizeF sizeHint(
        const QskSkinnable* skinnable, Qt::SizeHint which, const QSizeF& ) const override;
};