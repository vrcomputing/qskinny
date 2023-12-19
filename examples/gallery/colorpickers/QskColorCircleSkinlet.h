#ifndef QSK_COLOR_CIRCLE_SKINLET_H_
#define QSK_COLOR_CIRCLE_SKINLET_H_

#include <QskSkinlet.h>

class QskColorCircleSkinlet : public QskSkinlet
{
    Q_GADGET
  public:
    enum NodeRoles
    {
        CircleRole,
        HandleRole
    };

    Q_INVOKABLE QskColorCircleSkinlet( QskSkin* skin = nullptr );

  private:
    QRectF subControlRect( const QskSkinnable* skinnable, const QRectF&,
        QskAspect::Subcontrol subcontrol ) const override;

    QSGNode* updateSubNode(
        const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* node ) const override;

    QSizeF sizeHint(
        const QskSkinnable* skinnable, Qt::SizeHint which, const QSizeF& ) const override;
};

#endif