#ifndef QJOYPAD_LAYOUT_EDIT_H
#define QJOYPAD_LAYOUT_EDIT_H

#include <QAction>
#include <QMainWindow>
#include <QScrollArea>
#include <QStackedWidget>

//for the tab list of joypads
#include "flash.h"
//this is a front end for the non-GUI LayoutManager
class LayoutEdit;
#include "layout.h"
//so we can use the LayoutEditer to edit key layouts  :)
#include "joypadw.h"

#include "setting.hpp"

class LayoutManager;

typedef QPointer<QAction> QActionPtr;

class LayoutEdit : public QMainWindow {
	Q_OBJECT
	public:
        LayoutEdit( LayoutManager* l );
		//swap to a new layout
        void setLayout(const QString& layout);
		//update the list of available layouts
		void updateLayoutList();
        void updateJoypadWidgets();
    void setActionSetting( Setting::Enum e, bool value );

signals:
    void settingChanged( Setting::Enum e, bool value );

		void focusStateChanged(bool);
    public slots:
        void appFocusChanged(QWidget *old, QWidget *now);
private slots:
    void load(int index);
    // clears layout of pad settings of top widget stack;
    void clearTopPadLayout();
    // casts a magic spell on layout of pad settings of top widget stack;
    void wizardTopPadLayout();

    // some actions should be disabled on [NO LAYOUT] combobox
    void enableCertainActions( bool );

    void settingActionTriggered();

protected:
    QWidget* m_centralWidget;
		//the layout manager this represents
        LayoutManager* lm;
		//parts of the dialog:
        QVBoxLayout *mainLayout;
        QScrollArea *padScroll;
        QStackedWidget *padStack;
        FlashRadioArray *joyButtons;
        QComboBox* cmbLayouts;

    QPointer<QToolBar> m_toolBar;

    QActionPtr m_actionCloseWindow;
    QActionPtr m_actionQuit;
    // ----
    QActionPtr m_actionNewLayout;
    QActionPtr m_actionRemoveLayout;
    // ----
    QActionPtr m_actionOpen;
    QActionPtr m_actionSave;
    QActionPtr m_actionSaveAs;
    QActionPtr m_actionRename;
    // ----
    QActionPtr m_actionClear;
    QActionPtr m_actionQuickSet;
    QActionPtr m_actionRevert;
    // ----
    QActionPtr m_actionUseThemeTrayIcon;
    QActionPtr m_actionShowMenuBar;
    QActionPtr m_actionShowToolBar;

};

#endif
