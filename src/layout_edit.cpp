#include "layout_edit.h"
#include "config.h"

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>

#include <cassert>

//build the dialog
LayoutEdit::LayoutEdit( LayoutManager* l ) :
    QMainWindow( 0 ),
    m_centralWidget( new QWidget( 0 ) ),
      lm(l),
      mainLayout(0),
      padStack(0),
      joyButtons(0),
      cmbLayouts(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle( QJOYPAD_NAME );
    setWindowIcon(QPixmap(QJOYPAD_ICON64));

    setCentralWidget( m_centralWidget );
    mainLayout = new QVBoxLayout( m_centralWidget );
    mainLayout->setSpacing(5);
    mainLayout->setMargin(5);

    cmbLayouts = new QComboBox( 0 );
    connect(cmbLayouts, SIGNAL(activated(int)), this, SLOT(load(int)));
    mainLayout->addWidget( cmbLayouts );

    //produce a list of names for the FlashRadioArray
    //this is only necesary since joystick devices need not always be
    //contiguous
    QStringList names;
    foreach (JoyPad *joypad, lm->available) {
        names.append(joypad->getName());
        connect(this, SIGNAL(focusStateChanged(bool)), joypad, SLOT(focusChange(bool)));
    }

    //flash radio array
    joyButtons = new FlashRadioArray(names, true, 0 );
    mainLayout->addWidget( joyButtons );

    //we have a WidgetStack to represent the multiple joypads
    padScroll = new QScrollArea( 0 );
    padScroll->setWidgetResizable(true);
    mainLayout->addWidget(padScroll);
    padStack = new QStackedWidget( 0 );
    padScroll->setWidget(padStack);

    //go through each of the available joysticks
    // i is the current index into PadStack
    int i = 0;
    foreach (JoyPad *joypad, lm->available) {
        //add a new JoyPadWidget to the stack
        padStack->insertWidget( i, joypad->widget(padStack,i) );
        //every time it "flashes", flash the associated tab.
        connect( padStack->widget(i), SIGNAL( flashed( int ) ), joyButtons, SLOT( flash( int )));
        ++i;
    }
    //whenever a new tab is selected, raise the appropriate JoyPadWidget
    connect( joyButtons, SIGNAL( changed( int ) ), padStack, SLOT( setCurrentIndex( int )));

    updateLayoutList();
    connect(qApp, SIGNAL(focusChanged ( QWidget * , QWidget *  ) ), this, 
        SLOT(appFocusChanged(QWidget *, QWidget *)));

    m_actionCloseWindow = new QAction( QIcon::fromTheme( "window-close" ), tr( "Close window" ) );
    m_actionCloseWindow->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_W ) );
    connect( m_actionCloseWindow, &QAction::triggered, this, &LayoutEdit::close );
    m_actionQuit = new QAction( QIcon::fromTheme( "application-exit" ), tr( "Quit" ) );
    m_actionQuit->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_Q ) );
    connect( m_actionQuit, &QAction::triggered, lm, &LayoutManager::requestQuit );

    m_actionNewLayout = new QAction( QIcon::fromTheme( "document-new" ), tr( "New layout" ) );
    m_actionNewLayout->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_N ) );
    connect( m_actionNewLayout, &QAction::triggered, lm, &LayoutManager::saveAs );
    m_actionRemoveLayout = new QAction( QIcon::fromTheme( "document-close" ), tr( "Remove layout" ) );
    connect( m_actionRemoveLayout, &QAction::triggered, lm, &LayoutManager::remove );

    m_actionOpen = new QAction( QIcon::fromTheme( "document-open" ), tr( "Open" ) );
    m_actionOpen->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_O ) );
    connect( m_actionOpen, &QAction::triggered, lm, &LayoutManager::importLayout );
    m_actionSave = new QAction( QIcon::fromTheme( "document-save" ), tr( "Save" ) );
    m_actionSave->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_S ) );
    connect( m_actionSave, SIGNAL( triggered( bool ) ), lm, SLOT( save() ) );
    m_actionSaveAs = new QAction( QIcon::fromTheme( "document-save-as" ), tr( "Save as" ) );
    m_actionSaveAs->setShortcut( QKeySequence( Qt::CTRL | Qt::SHIFT | Qt::Key_S ) );
    connect( m_actionSaveAs, &QAction::triggered, lm, &LayoutManager::exportLayout );
    m_actionRename = new QAction( QIcon::fromTheme( "edit-rename" ), tr( "Rename" ) );
    connect( m_actionRename, &QAction::triggered, lm, &LayoutManager::rename );
    m_actionClear = new QAction( QIcon::fromTheme( "edit-clear" ), tr( "Clear" ) );
    connect( m_actionClear, &QAction::triggered, this, &LayoutEdit::clearTopPadLayout );
    m_actionQuickSet = new QAction( QIcon::fromTheme( "tools-wizard" ), tr( "Quick set" ) );
    connect( m_actionQuickSet, &QAction::triggered, this, &LayoutEdit::wizardTopPadLayout );
    m_actionRevert = new QAction( QIcon::fromTheme( "document-revert" ), tr( "Revert changes" ) );
    connect( m_actionRevert, &QAction::triggered, lm, &LayoutManager::reload );

    m_actionUseThemeTrayIcon = new QAction( tr( "Use tray icon from theme" ) );
    m_actionUseThemeTrayIcon->setCheckable( true );
    m_actionUseThemeTrayIcon->setData( Setting::useTrayIconFromTheme );
    connect( m_actionUseThemeTrayIcon, &QAction::triggered, this, &LayoutEdit::settingActionTriggered );
    m_actionShowMenuBar = new QAction( tr( "Show menubar" ) );
    m_actionShowMenuBar->setCheckable( true );
    m_actionShowMenuBar->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_M ) );
    m_actionShowMenuBar->setData( Setting::showMenuBar );
    connect( m_actionShowMenuBar, &QAction::triggered, this, &LayoutEdit::settingActionTriggered );
    m_actionShowToolBar = new QAction( tr( "Show toolbar" ) );
    m_actionShowToolBar->setCheckable( true );
    m_actionShowToolBar->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_T ) );
    m_actionShowToolBar->setData( Setting::showToolBar );
    connect( m_actionShowToolBar, &QAction::triggered, this, &LayoutEdit::settingActionTriggered );

    QMenuBar* menuBar = new QMenuBar( 0 );
    setMenuBar( menuBar );
    // out of better name for this entry
    QMenu* menu = menuBar->addMenu( "QJoyPad" );
    menu->addAction( m_actionCloseWindow );
    menu->addAction( m_actionQuit );
    menu = menuBar->addMenu( tr( "Layout" ) );
    menu->addAction( m_actionNewLayout );
    menu->addAction( m_actionRemoveLayout );
    menu->addSeparator();
    menu->addAction( m_actionOpen );
    menu->addAction( m_actionSave );
    menu->addAction( m_actionSaveAs );
    menu->addAction( m_actionRename );
    menu->addSeparator();
    menu->addAction( m_actionClear );
    menu->addAction( m_actionQuickSet );
    menu->addAction( m_actionRevert );
    menu = menuBar->addMenu( tr( "View" ) );
    menu->addAction( m_actionUseThemeTrayIcon );
    menu->addAction( m_actionShowMenuBar );
    menu->addAction( m_actionShowToolBar );

    m_toolBar = new QToolBar( 0 );
    addToolBar( m_toolBar );
    m_toolBar->addAction( m_actionNewLayout );
    m_toolBar->addAction( m_actionRemoveLayout );
    m_toolBar->addSeparator();
    m_toolBar->addAction( m_actionOpen );
    m_toolBar->addAction( m_actionSave );
    m_toolBar->addAction( m_actionSaveAs );
    m_toolBar->addAction( m_actionRename );
    m_toolBar->addSeparator();
    m_toolBar->addAction( m_actionClear );
    m_toolBar->addAction( m_actionQuickSet );
    m_toolBar->addAction( m_actionRevert );

    // setting window shortcuts, the action can be triggered only
    // if the corresponding menubar/toolbar is visible. In case
    // when the menubar/toolbar is not visible, we still want to
    // have shortcuts working
    addAction( m_actionCloseWindow );
    addAction( m_actionQuit );
    addAction( m_actionNewLayout );
    addAction( m_actionOpen );
    addAction( m_actionSave );
    addAction( m_actionSaveAs );
    addAction( m_actionShowMenuBar );
    addAction( m_actionShowToolBar );

    show();
}

void LayoutEdit::setLayout(const QString &layout) {
    //change the selection
    for (int i = 0; i < cmbLayouts->count(); ++ i) {
        if (cmbLayouts->itemData(i).toString() == layout) {
            cmbLayouts->setCurrentIndex(i);
            break;
        }
    }

    //update all the JoyPadWidgets.
    for (int i = 0, n = lm->available.count(); i < n; i++) {
        ((JoyPadWidget*)padStack->widget(i))->update();
    }
}

void LayoutEdit::updateLayoutList() {
    //blank the list, then load in new names from the LayoutManager.
    cmbLayouts->clear();
    cmbLayouts->addItem(tr("[NO LAYOUT]"), QVariant(QString::null));
    if (lm->currentLayout.isNull()) {
        cmbLayouts->setCurrentIndex(0);
    }
    foreach (const QString& layout, lm->getLayoutNames()) {
        cmbLayouts->addItem(layout, layout);
        if (layout == lm->currentLayout) {
            cmbLayouts->setCurrentIndex(cmbLayouts->count() - 1);
        }
    }
}

void LayoutEdit::updateJoypadWidgets() {
    int indexOfFlashRadio = mainLayout->indexOf(joyButtons);
    FlashRadioArray *newJoyButtons;
    QStringList names;
    foreach (JoyPad *joypad, lm->available) {
        names.append(joypad->getName());
    }
    
    newJoyButtons = new FlashRadioArray( names, true, this );
    mainLayout->insertWidget(indexOfFlashRadio, newJoyButtons);
    mainLayout->removeWidget(joyButtons);
    FlashRadioArray* oldJoyButtons = joyButtons;
    joyButtons = newJoyButtons;
    connect( joyButtons, SIGNAL( changed( int ) ), padStack, SLOT( setCurrentIndex( int )));
    oldJoyButtons->deleteLater();
    int numberOfJoypads = padStack->count();
    for(int i = 0; i<numberOfJoypads; i++) {
        padStack->removeWidget(padStack->widget(0));
    }
    int i = 0;
    foreach (JoyPad *joypad, lm->available) {
        //add a new JoyPadWidget to the stack
        padStack->insertWidget( i, joypad->widget(padStack,i) );
        //every time it "flashes", flash the associated tab.
        connect( padStack->widget(i), SIGNAL( flashed( int ) ), joyButtons, SLOT( flash( int )));
        ++i;
    }
}

void LayoutEdit::appFocusChanged(QWidget *old, QWidget *now) {
    if (now != NULL && old == NULL) {
        emit focusStateChanged(false);
    } else if(old != NULL && now == NULL) {
        emit focusStateChanged(true);
        foreach (JoyPad *joypad, lm->available) {
            debug_mesg("iterating and releasing\n");
            joypad->release();
        }
        debug_mesg("done releasing!\n");
    }
}

void LayoutEdit::load(int index) {
    lm->load(cmbLayouts->itemData(index).toString());
    emit enableCertainActions( index != 0 );
}

void LayoutEdit::clearTopPadLayout()
{
    QPointer<JoyPadWidget> widgetPtr = qobject_cast<JoyPadWidget*>( padStack->currentWidget() );
    if ( !widgetPtr ) {
        return;
    }
    widgetPtr->clear();
}

void LayoutEdit::wizardTopPadLayout()
{
    QPointer<JoyPadWidget> widgetPtr = qobject_cast<JoyPadWidget*>( padStack->currentWidget() );
    if ( !widgetPtr ) {
        return;
    }
    widgetPtr->setAll();
}

// for a case of current layout being [NO LAYOUT]
void LayoutEdit::enableCertainActions( bool b )
{
    m_actionRemoveLayout->setEnabled( b );
    m_actionRename->setEnabled( b );
}

void LayoutEdit::setActionSetting( Setting::Enum e, bool value )
{
    switch ( e ) {
        case Setting::showMenuBar:
            m_actionShowMenuBar->setChecked( value );
            break;
        case Setting::showToolBar:
            m_actionShowToolBar->setChecked( value );
            break;
        case Setting::useTrayIconFromTheme:
            m_actionUseThemeTrayIcon->setChecked( value );
            break;
        default:
            assert( !"unknown setting" );
    }
}

void LayoutEdit::settingActionTriggered()
{
    const QAction* action = qobject_cast<QAction*>( QObject::sender() );
    assert( action );
    const Setting::Enum e = static_cast<Setting::Enum>( action->data().toInt() );
    const bool checked = action->isChecked();
    switch ( e ) {
        case Setting::showMenuBar:
            menuBar()->setVisible( checked );
            break;
        case Setting::showToolBar:
            m_toolBar->setVisible( checked );
            break;
        case Setting::useTrayIconFromTheme:
            break;
        default:
            assert( !"unknown setting" );
    }
    emit settingChanged( e, checked );
}
