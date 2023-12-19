#ifndef QSK_COLOR_CIRCLE_H_
#define QSK_COLOR_CIRCLE_H_

#include "QskAspect.h"
#include <QskControl.h>
#include <qquickitem.h>

class QskColorCircle : public QskControl
{
    Q_OBJECT
  public:
    QSK_SUBCONTROLS( Circle, Handle )
    explicit QskColorCircle( QQuickItem* parent = nullptr );

    QColor selectedColor() const;

  protected:
    void mousePressEvent( QMouseEvent* event ) override;
    void mouseReleaseEvent( QMouseEvent* event ) override;
    void mouseMoveEvent( QMouseEvent* event ) override;

  Q_SIGNALS:
    void selectedColorChanged( const QColor& color );

  private:
    void setSelectedColor( const QColor& color );
    QColor m_selectedColor;
};

#endif