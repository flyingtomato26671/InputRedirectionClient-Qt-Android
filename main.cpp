#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include "mainwidget.h"
#include "gpmanager.h"
#include "global.h"
#include <QTimer>
#include <QThread>
#include <QStyleFactory>
#include <QStyle>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef Q_OS_ANDROID
    // El estilo "android" extraido del tema del sistema deja los widgets
    // invisibles en Android modernos; Fusion se dibuja siempre igual.
    a.setStyle(QStyleFactory::create("Fusion"));
#endif
    qWarning("IRC: QApplication ok, style=%s", qPrintable(a.style()->objectName()));
    Widget w;
    qWarning("IRC: Widget creado, minimumSizeHint=%dx%d", w.minimumSizeHint().width(), w.minimumSizeHint().height());
    GamepadMonitor m(&w);

    w.show();
    qWarning("IRC: show() hecho, size=%dx%d visible=%d", w.width(), w.height(), (int)w.isVisible());
    QThread* thread = new QThread();
    QTimer timer;
    timer.setInterval(20);
    timer.moveToThread(thread);

    Worker* worker = new Worker();
    worker->moveToThread(thread);

    QObject::connect(thread, SIGNAL (started()), &timer, SLOT (start()));
    QObject::connect(worker, SIGNAL (finished()), thread, SLOT (quit()));
    QObject::connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));
    QObject::connect(worker, SIGNAL (finished()), worker, SLOT (deleteLater()));
    QObject::connect(&timer, SIGNAL (timeout()), worker, SLOT(sendFrame()));
    thread->start();
    qWarning("IRC: entrando a exec()");
    a.exec();
    worker->closeThread();
    return 0;
}
