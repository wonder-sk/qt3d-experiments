#include <Qt3DQuickExtras/qt3dquickwindow.h>
#include <Qt3DQuick/QQmlAspectEngine>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>


int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    // TODO: not sure how to set surface format for Qt3DQuickWindow in order
    // to set up number of samples per pixel for multisample anti-aliasing
    // (calling view.setFormat(...) with changed number of samples caused OpenGL context creation failures)

    Qt3DExtras::Quick::Qt3DQuickWindow view;
    view.setTitle("Multisample anti-aliasing (MSAA)");
    view.resize(1600, 800);
    view.engine()->qmlEngine()->rootContext()->setContextProperty("_window", &view);
    view.engine()->qmlEngine()->rootContext()->setContextProperty("_dpr", view.devicePixelRatio());
    view.setSource(QUrl("qrc:/main.qml"));
    view.show();

    return app.exec();
}
