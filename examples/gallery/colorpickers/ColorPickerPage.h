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

class ColorPickerPage : public Page
{
  public:
    explicit ColorPickerPage( QQuickItem* = nullptr );
};

class QskColorWheel : public QskControl
{
  public:
    QSK_SUBCONTROLS( Panel, Handle )
    explicit QskColorWheel( QQuickItem* parent = nullptr );

    void mousePressEvent( QMouseEvent* event ) override;
    void mouseReleaseEvent( QMouseEvent* event ) override;
    void mouseMoveEvent( QMouseEvent* event ) override;
};

class QskColorWheelSkinlet : public QskSkinlet
{
  public:
    enum NodeRoles
    {
        PanelRole
    };

    QskColorWheelSkinlet( QskSkin* skin = nullptr );

    QRectF subControlRect(
        const QskSkinnable* skinnable, const QRectF&, QskAspect::Subcontrol ) const override;

    QSGNode* updateSubNode(
        const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* ) const override;

    QSizeF sizeHint(
        const QskSkinnable* skinnable, Qt::SizeHint which, const QSizeF& ) const override;
};