#ifndef QSK_COLOR_TRIANGLE_SKINLET_H_
#define QSK_COLOR_TRIANGLE_SKINLET_H_

#include <QskSkinlet.h>

class QskColorTriangleSkinlet : public QskSkinlet
{
    Q_GADGET
  public:
    enum NodeRoles
    {
        TriangleRole,
        HandleRole
    };

    Q_INVOKABLE QskColorTriangleSkinlet( QskSkin* skin = nullptr );

  private:
    QRectF subControlRect(
        const QskSkinnable* skinnable, const QRectF&, QskAspect::Subcontrol ) const override;

    QSGNode* updateSubNode(
        const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* ) const override;

    QSizeF sizeHint(
        const QskSkinnable* skinnable, Qt::SizeHint which, const QSizeF& ) const override;
};

#endif
