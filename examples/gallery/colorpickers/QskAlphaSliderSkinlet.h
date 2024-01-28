#include <QskSliderSkinlet.h>

class QskAlphaSliderSkinlet : public QskSliderSkinlet
{
    using Inherited = QskSliderSkinlet;
public:
    enum NodeRoles { GridRole = Inherited::RoleCount, RoleCount };
    Q_INVOKABLE QskAlphaSliderSkinlet(QskSkin* skin = nullptr);
protected:
    QSGNode* updateSubNode( const QskSkinnable* skinnable, quint8 nodeRole, QSGNode* node) const override;
};