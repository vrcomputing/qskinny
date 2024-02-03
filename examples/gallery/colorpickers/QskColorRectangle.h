#pragma once

#include <QColor>
#include <QskControl.h>

class QskColorRectangle : public QskControl
{
    Q_OBJECT
    Q_PROPERTY( QColor selectedColor READ selectedColor NOTIFY selectedColorChanged )

  public:
    QSK_SUBCONTROLS( Panel, Handle )
    explicit QskColorRectangle( QQuickItem* parent = nullptr );
    
    QColor selectedColor() const;

    void setCornerColor( Qt::Corner corner, const QColor& color );
    QColor cornerColor( Qt::Corner corner ) const;

  protected:
    void mousePressEvent( QMouseEvent* event ) override;
    void mouseReleaseEvent( QMouseEvent* event ) override;
    void mouseMoveEvent( QMouseEvent* event ) override;

  Q_SIGNALS:
    void selectedColorChanged( const QColor& color );

  private:
    void setSelectedColor( const QColor& color );

    QColor m_selectedColor;
    QColor m_cornerColor[ 4 ];
};