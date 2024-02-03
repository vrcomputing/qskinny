#ifndef QSK_COLOR_WHEEL_H_
#define QSK_COLOR_WHEEL_H_

#include <QskControl.h>

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

#endif