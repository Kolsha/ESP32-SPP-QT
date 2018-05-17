#include <QApplication>
#include "chat.h"






int main(int argc, char *argv[])
{
    //QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QApplication app(argc, argv);

    Chat d;
    QObject::connect(&d, SIGNAL(accepted()), &app, SLOT(quit()));

    d.show();

    app.exec();

    return 0;
}

