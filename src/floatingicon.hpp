#pragma once

#include <QDialog>
#include <QMenu>
#include <QPixmap>
#include <QPointer>
#include <QMouseEvent>
#include <QCloseEvent>


class FloatingIcon : public QDialog {
    Q_OBJECT

public:
    FloatingIcon();
    void setIcon( const QIcon& icon );
    void setContextMenu( QMenu* popup );

protected:
    void paintEvent( QPaintEvent* );
    void mousePressEvent( QMouseEvent* );
    void closeEvent( QCloseEvent* );

private:
    QPointer<QMenu> m_menu;
    QPixmap m_pixmap;

signals:
    void clicked();
    void quit();
};
