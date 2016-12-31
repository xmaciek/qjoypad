#include "floatingicon.hpp"

#include <cassert>
#include <QPainter>

#include "config.h"
#include "constant.h"

FloatingIcon::FloatingIcon()
{
    setAttribute( Qt::WA_DeleteOnClose );
    setAttribute( Qt::WA_TranslucentBackground );
    setWindowFlags( Qt::FramelessWindowHint );
    setWindowTitle( tr( "%1 Floating Icon" ).arg( QJOYPAD_NAME ) );
}

void FloatingIcon::mousePressEvent( QMouseEvent* event ) {
    //if it was the right mouse button,
    if (event->button() == Qt::RightButton) {
        //bring up the popup menu.
        assert( m_menu );
        m_menu->popup( event->globalPos() );
    }
    else {
        //otherwise, treat it as a regular click.
        emit clicked();
    }
}

void FloatingIcon::setIcon( const QIcon& icon )
{
    const QSize size( 64, 64 );
    m_pixmap = icon.pixmap( size );
    setFixedSize( size );
}

void FloatingIcon::setContextMenu( QMenu* menu )
{
    m_menu = menu;
}

void FloatingIcon::paintEvent( QPaintEvent* ) {
    QPainter painter(this);
    painter.drawPixmap( 0, 0, m_pixmap );
}

// not sure if bug/unknown Qt feature or problem at my end,
// but the closeEvent never fires with Qt::FramelessWindowHint flag set.
void FloatingIcon::closeEvent( QCloseEvent* event )
{
    emit quit();
    QDialog::closeEvent( event );
}
