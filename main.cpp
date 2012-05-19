#include "QNcidNotify.h"

#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/QSystemTrayIcon>

 int main(int argc, char *argv[])
 {
     Q_INIT_RESOURCE(QNcidNotify);

     QApplication app(argc, argv);

     if (!QSystemTrayIcon::isSystemTrayAvailable()) {
         QMessageBox::critical(0, QObject::tr("Systray"),
                               QObject::tr("I couldn't detect any system tray "
                                           "on this system."));
         return 1;
     }
     QApplication::setQuitOnLastWindowClosed(false);

     QNcidNotify window;
     window.show();
     return app.exec();
 }