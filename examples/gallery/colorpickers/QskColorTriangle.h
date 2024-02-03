#ifndef QSK_COLOR_TRIANGLE_H_
#define QSK_COLOR_TRIANGLE_H_

#include <QskControl.h>

class QskColorTriangle : public QskControl
{
    Q_OBJECT
    Q_PROPERTY(
        QColor selectedColor READ selectedColor WRITE setSelectedColor NOTIFY selectedColorChanged )

  public:
    QSK_SUBCONTROLS( Triangle, Handle )
    explicit QskColorTriangle( QQuickItem* parent = nullptr );

    QColor selectedColor() const;

    void setColors( const QColor& c1, const QColor& c2, const QColor& c3 );
    QColor color1() const;
    QColor color2() const;
    QColor color3() const;

  Q_SIGNALS:
    void selectedColorChanged( const QColor& color );

  protected:
    void mousePressEvent( QMouseEvent* event ) override;
    void mouseReleaseEvent( QMouseEvent* event ) override;
    void mouseMoveEvent( QMouseEvent* event ) override;

  private:
    void setSelectedColor( const QColor& color );

    QColor m_selectedColor;
    QColor m_c1 = Qt::transparent;
    QColor m_c2 = Qt::black;
    QColor m_c3 = Qt::white;
};

#endif