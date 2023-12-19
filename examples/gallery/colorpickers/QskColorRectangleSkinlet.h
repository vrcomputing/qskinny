#pragma once

#include <QskSkinlet.h>

class QskColorRectangleSkinlet : public QskSkinlet
{
    Q_GADGET
  public:
    enum NodeRoles
    {
        PanelRole,
        HandleRole,
        RoleCount
    };
    Q_INVOKABLE QskColorRectangleSkinlet( QskSkin* skin = nullptr );

  private:
    QRectF subControlRect( const QskSkinnable* skinnable, const QRectF& contentsRect,
        QskAspect::Subcontrol subcontrol ) const;

    QSGNode* updateSubNode( const QskSkinnable* skinnable,
        quint8 nodeRole, QSGNode* node) const;
};