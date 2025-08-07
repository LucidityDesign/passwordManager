#include "ui/mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <sodium.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (sodium_init() < 0)
    {
        qFatal("libsodium initialization failed!");
    }

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "passwordmanager_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
