#pragma once

#include <QIcon>
#include <QMenu>
#include <QPointer>
#include <QObject>

class TrayIcon : public QObject {
    Q_OBJECT

public:
    enum Type { Unknown, Floating, System };
    TrayIcon( Type e );
    void setIcon( const QIcon& icon );
    void setContextMenu( QMenu* menu );
    void show();

private:
    QPointer<QObject> m_trayPtr;
    Type m_type;

private slots:
    void trayClick( /*QSystemTrayIcon::ActivationReason*/ int );

signals:
    void clicked();
    void quit();
};
