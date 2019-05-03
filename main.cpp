#include <QGuiApplication>
#include <QQuickView>
#include <QQmlEngine>

#include "framehandler.h"

int main(int argc, char* argv[])
{
    QGuiApplication::setApplicationName("App");
    QGuiApplication app(argc,argv);

    QSurfaceFormat defaultFormat;
    defaultFormat.setProfile(QSurfaceFormat::CoreProfile);
    defaultFormat.setRenderableType(QSurfaceFormat::OpenGLES);
    defaultFormat.setVersion(3, 0);
    defaultFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(defaultFormat);

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
