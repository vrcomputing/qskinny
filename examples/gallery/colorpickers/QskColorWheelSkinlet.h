#ifndef QSK_COLOR_WHEEL_SKINLET_H_
#define QSK_COLOR_WHEEL_SKINLET_H_

#include <QskSkinlet.h>
#include <qobjectdefs.h>

class QskColorWheelSkinlet : public QskSkinlet
{
    Q_GADGET
  public:
    enum NodeRoles
    {
        ArcRole,
        HandleRole
    };

    Q_INVOKABLE QskColorWheelSkinlet( QskSkin* skin = nullptr );

  private:
    QRectF subControlRect(
        const QskSkinnable* skinnable, const QRectF&, QskAspect::Subcontrol ) const override;

    QSGNode* updateSubNode(
        const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* ) const override;

    QSizeF sizeHint(
        const QskSkinnable* skinnable, Qt::SizeHint which, const QSizeF& ) const override;
};

#endif