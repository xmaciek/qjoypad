#include "trayicon.hpp"

#include <cassert>

#include <QSystemTrayIcon>
#include "floatingicon.hpp"

TrayIcon::TrayIcon( Type t ) :
    m_type( t )
{
    switch ( m_type ) {
        case TrayIcon::Floating: {
            FloatingIcon* ptr = new FloatingIcon();
            m_trayPtr = ptr;
            connect( ptr, &FloatingIcon::clicked, this, &TrayIcon::clicked );
            connect( ptr, &FloatingIcon::quit, this, &TrayIcon::quit );
        } break;
        case TrayIcon::System: {
            QSystemTrayIcon* ptr = new QSystemTrayIcon();
            m_trayPtr = ptr;
            connect( ptr, &QSystemTrayIcon::activated, this, &TrayIcon::trayClick );
        } break;
        default:
            assert( !"unknown tray type" );
    }
}

void TrayIcon::trayClick( /*QSystemTrayIcon::ActivationReason*/ int reason )
{
    if ( reason == QSystemTrayIcon::Trigger ) {
        emit clicked();
    }
}

void TrayIcon::setIcon( const QIcon& icon )
{
    assert( m_trayPtr );
    switch ( m_type ) {
        case TrayIcon::Floating:
            qobject_cast<FloatingIcon*>( m_trayPtr )->setIcon( icon );
            qobject_cast<FloatingIcon*>( m_trayPtr )->update();
            break;
        case TrayIcon::System:
            qobject_cast<QSystemTrayIcon*>( m_trayPtr )->setIcon( icon );
            break;
        default:
            assert( !"unknown tray type" );
    }
}

void TrayIcon::setContextMenu( QMenu* menu )
{
    assert( m_trayPtr );
    switch ( m_type ) {
        case TrayIcon::Floating:
            qobject_cast<FloatingIcon*>( m_trayPtr )->setContextMenu( menu );
            break;
        case TrayIcon::System:
            qobject_cast<QSystemTrayIcon*>( m_trayPtr )->setContextMenu( menu );
            break;
        default:
            assert( !"unknown tray type" );
    }
}

void TrayIcon::show()
{
    assert( m_trayPtr );
    switch ( m_type ) {
        case TrayIcon::Floating:
            qobject_cast<FloatingIcon*>( m_trayPtr )->show();
            break;
        case TrayIcon::System:
            qobject_cast<QSystemTrayIcon*>( m_trayPtr )->show();
            break;
        default:
            assert( !"unknown tray type" );
    }
}
