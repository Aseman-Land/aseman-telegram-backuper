#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("TelegramBackuper");
    app.setOrganizationName("Aseman Team");
    app.setApplicationVersion("0.8");
    app.setApplicationDisplayName("Aseman Telegram Backuper");
    app.setOrganizationDomain("io.aseman");
    app.setWindowIcon( QIcon(":/icons/icon.png") );

    qputenv("QT_LOGGING_RULES", "tg.*=false");

    MainWindow w;
    w.setWindowIcon(app.windowIcon());
    w.show();

    return app.exec();
}
