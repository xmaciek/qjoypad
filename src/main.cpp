//for ouput when there is no GUI going
#include <stdio.h>
#include <unistd.h>
//to create and handle signals for various events
#include <signal.h>
#include <getopt.h>

//to create a qapplication
#include <QFile>
#include <QSystemTrayIcon>
#include <QPointer>
#include <QFileInfo>
#include <QTranslator>

//to load layouts
#include "layout.h"
//to give event.h the current X11 display
#include "event.h"
//to produce errors!
#include "error.h"
#include "config.h"

//variables needed in various functions in this file
QPointer<LayoutManager> layoutManagerPtr;

//signal handler for SIGUSR2
//SIGUSR2 means that a new layout should be loaded. It is saved in
// ~/.qjoypad/layout, where the last used layout is put.
void catchSIGUSR2( int sig ) {
    if (layoutManagerPtr) layoutManagerPtr->load();
    //remember to catch this signal again next time.
    signal( sig, catchSIGUSR2 );
}



//signal handler for SIGUSR1
//SIGUSR1 means that we should update the available joystick device list.
void catchSIGUSR1( int sig ) {
    //buildJoyDevices();
    if (layoutManagerPtr) layoutManagerPtr->updateJoyDevs();
    //remember to catch this signal again next time.
    signal( sig, catchSIGUSR1 );
}


int main( int argc, char **argv )
{
    //create a new event loop. This will be captured by the QApplication
    //when it gets created
    QApplication app( argc, argv );
    QTranslator translator;
    app.setQuitOnLastWindowClosed(false);

    if (translator.load(QLocale::system(), "qjoypad", "_", QJOYPAD_L10N_DIR)) {
        app.installTranslator(&translator);
    }
    else {
        debug_mesg("no translation for locale: %s\n", qPrintable(QLocale::system().name()));
    }


    //where QJoyPad saves its settings!
    const QString settingsDir(QDir::homePath() + "/.qjoypad3/");

    //where to look for settings. If it does not exist, it will be created
    QDir dir(settingsDir);

    //the directory in wich the joystick devices are (e.g. "/dev/input")
    QString devdir = QJOYPAD_DEVDIR;

    //if there is no new directory and we can't make it, complain
    if (!dir.exists() && !dir.mkdir(settingsDir)) {
        errorBox(app.translate("main","Couldn't create the QJoyPad save directory"),
                 app.translate("main","Couldn't create the QJoyPad save directory: %s").arg(settingsDir));
        return 1;
    }


    //start out with no special layout.
    QString layout;
    //by default, we use a tray icon
    bool useTrayIcon = true;
    //this execution wasn't made to update the joystick device list.
    bool update = false;
    bool forceTrayIcon = false;

    //parse command-line options
    struct option long_options[] = {
        {"help",       no_argument,       0, 'h'},
        {"device",     required_argument, 0, 'd'},
        {"force-tray", no_argument,       0, 't'},
        {"notray",     no_argument,       0, 'T'},
        {"update",     no_argument,       0, 'u'},
        {0,            0,                 0,  0 }
    };

    for (;;) {
        int c = getopt_long(argc, argv, "hd:tTu", long_options, NULL);

        if (c == -1)
            break;

        switch (c) {
            case 'h':
                printf("%s", qPrintable(app.translate("main","%1\n"
                    "Usage: %2 [--device=\"/device/path\"] [--notray|--force-tray] [\"layout name\"]\n"
                    "\n"
                    "Options:\n"
                    "  -h, --help            Print this help message.\n"
                    "  -d, --device=PATH     Look for joystick devices in PATH. This should\n"
                    "                        be something like \"/dev/input\" if your game\n"
                    "                        devices are in /dev/input/js0, /dev/input/js1, etc.\n"
                    "  -t, --force-tray      Force to use a system tray icon.\n"
                    "  -T, --notray          Do not use a system tray icon. This is useful for\n"
                    "                        window managers that don't support this feature.\n"
                    "  -u, --update          Force a running instance of QJoyPad to update its\n"
                    "                        list of devices and layouts.\n"
                    "  \"layout name\"         Load the given layout in an already running\n"
                    "                        instance of QJoyPad, or start QJoyPad using the\n"
                    "                        given layout.\n").arg(QJOYPAD_NAME, argc > 0 ? argv[0] : "qjoypad")));
                return 0;

            case 'd':
                if (QFileInfo(optarg).isDir()) {
                    devdir = optarg;
                }
                else {
                    errorBox(app.translate("main","Not a directory"),
                             app.translate("main","Path is not a directory: %1").arg(optarg));
                    return 1;
                }
                break;

            case 'T':
                useTrayIcon = false;
                break;

            case 't':
                useTrayIcon = true;
                forceTrayIcon = true;
                break;

            case 'u':
                update = true;
                break;

            case '?':
                fprintf(stderr, "%s", qPrintable(app.translate("main",
                    "Illeagal argument.\n"
                    "See `%1 --help` for more information\n").arg(argc > 0 ? argv[0] : "qjoypad")));
                return 1;
        }
    }

    if (optind < argc) {
        layout = argv[optind ++];

        if (optind < argc) {
            fprintf(stderr, "%s", qPrintable(app.translate("main",
                "Too many arguments.\n"
                "See `%1 --help` for more information\n").arg(argc > 0 ? argv[0] : "qjoypad")));
            return 1;
        }
    }

    //if the user specified a layout to use,
    if (!layout.isEmpty())
    {
        //then we try to store that layout in the last-used layout spot, to be
        //loaded by default.
        QFile file(settingsDir + "layout");
        if (file.open(QIODevice::WriteOnly))
        {
            QTextStream( &file ) << layout;
            file.close();
        }
    }





    //create a pid lock file.
    QFile pidFile( "/tmp/qjoypad.pid" );
    //if that file already exists, then qjoypad is already running!
    if (pidFile.exists())
    {
        int pid = 0;
        if (pidFile.open( QIODevice::ReadOnly ))
        {
            //try to get that pid...
            QTextStream( &pidFile ) >> pid;
            pidFile.close();
            //if we can signal the pid (ie, if the process is active)
            if (kill(pid,0) == 0)
            {
                //then prevent two instances from running at once.
                //however, if we are setting the layout or updating the device
                //list, this is not an error and we shouldn't make one!
                if (layout.isEmpty() && !update)
                    errorBox(app.translate("main","Instance Error"),
                             app.translate("main","There is already a running instance of QJoyPad; please close\nthe old instance before starting a new one."));
                else {
                    //if one of these is the case, send the approrpriate signal!
                    if (update) {
                        kill(pid,SIGUSR1);
                    }
                    if (!layout.isEmpty()) {
                        kill(pid,SIGUSR2);
                    }
                }
                //and quit. We don't need two instances.
                return 0;
            }
        }
    }
    //now we can try to create and write our pid to the lock file.
    if (pidFile.open( QIODevice::WriteOnly ))
    {
        QTextStream( &pidFile ) << getpid();
        pidFile.close();
    }

    if (forceTrayIcon) {
        int sleepCounter = 0;
        while (!QSystemTrayIcon::isSystemTrayAvailable()) {
            sleep(1);
            sleepCounter++;
            if (sleepCounter > 20) {
                errorBox(app.translate("main","System tray isn't loading"),
                         app.translate("main","Waited more than 20 seconds for the system tray to load. Giving up."));
                return 1;
            }
        }
    }
    //create a new LayoutManager with a tray icon / floating icon, depending
    //on the user's request
    LayoutManager layoutManager(useTrayIcon,devdir,settingsDir);
    layoutManagerPtr = &layoutManager;
    QObject::connect( &layoutManager, &LayoutManager::quit, &app, &QApplication::quit );

    //build the joystick device list for the first time,
    //buildJoyDevices();
    layoutManager.updateJoyDevs();
    
    //load the last used layout (Or the one given as a command-line argument)
    layoutManager.load();

    //prepare the signal handlers
    signal( SIGUSR1, catchSIGUSR1 );
    signal( SIGUSR2, catchSIGUSR2 );

    //and run the program!
    int result = app.exec();

    //remove the lock file...
    pidFile.remove();

    //and terminate!
    return result;
}
