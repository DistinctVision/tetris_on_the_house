#include <QGuiApplication>
#include <QQuickView>
#include <QQmlEngine>

#include "framehandler.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc,argv);
    qmlRegisterType<FrameHandler>("mystuffs", 1, 0, "FrameHandler");
    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    QObject::connect(view.engine(), &QQmlEngine::quit,
                     qApp, &QGuiApplication::quit);
    view.setSource(QUrl("qrc:///qml/MainView.qml"));
    view.resize(800, 480);
    view.show();
    return app.exec();
}
