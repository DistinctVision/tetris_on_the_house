#include <QGuiApplication>
#include <QQuickView>
#include <QQmlEngine>

#include "framehandler.h"
#include "gl/gl_view.h"
#include "gl/gl_scene.h"
#include "debugimageobject.h"
#include "gl/gl_debugimagescene.h"
#include "game/testcubescene.h"
#include "game/tetrisscene.h"
#include "objectedgestracker.h"
#include "texturereceiver.h"

void regsiterQmlTypes()
{
    qmlRegisterType<FrameHandler>("mystuffs", 1, 0, "FrameHandler");
    qmlRegisterUncreatableType<ObjectEdgesTracker>("mystuffs", 1, 0, "ObjectEdgesTracker", "!");
    qmlRegisterType<GL_View>("mystuffs", 1, 0, "GL_View");
    qmlRegisterUncreatableType<FillMode>("mystuffs", 1, 0, "FillMode", "Its enum");
    qmlRegisterUncreatableType<DebugImageObject>("mystuffs", 1, 0, "DebugImageObject", "Abstract class");
    qmlRegisterUncreatableType<GL_Scene>("mystuffs", 1, 0, "GL_Scene", "Abstract class");
    qmlRegisterType<GL_DebugImageScene>("mystuffs", 1, 0, "GL_DebugImageScene");
    qmlRegisterType<TestCubeScene>("mystuffs", 1, 0, "TestCubeScene");
    qmlRegisterType<TetrisScene>("mystuffs", 1, 0, "TetrisScene");
    qmlRegisterType<TextureReceiver>("mystuffs", 1, 0, "TextureReceiver");
    qmlRegisterUncreatableType<TrackingQuality>("mystuffs", 1, 0, "TrackingQuality", "It's enum");
}

int main(int argc, char* argv[])
{
    QGuiApplication app(argc,argv);
    app.setOrganizationName("App inc.");
    app.setOrganizationDomain("appinc.com");
    app.setApplicationName("App");

    /*QSurfaceFormat defaultFormat;
    defaultFormat.setProfile(QSurfaceFormat::CompatibilityProfile);
    //defaultFormat.setRenderableType(QSurfaceFormat::OpenGLES);
    defaultFormat.setVersion(3, 3);
    defaultFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(defaultFormat);*/

    regsiterQmlTypes();

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    QObject::connect(view.engine(), &QQmlEngine::quit,
                     qApp, &QGuiApplication::quit);
    view.setSource(QUrl("qrc:///qml/MainView.qml"));
    //view.resize(800, 480);
    view.show();
    return app.exec();
}
