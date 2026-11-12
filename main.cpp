#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>

#include "version.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qTarkovTracker_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    a.setWindowIcon(QIcon(":/icon.png"));
    a.setApplicationVersion(APP_VERSION);

    QFile file(":/resources/dark.qss");

    if (file.open(QIODevice::ReadOnly)) {
        a.setStyleSheet(QString::fromUtf8(file.readAll()));
    }

    MainWindow w;
    w.show();
    return QApplication::exec();
}
