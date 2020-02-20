#include <Qt3DQuickExtras/qt3dquickwindow.h>
#include <Qt3DQuick/QQmlAspectEngine>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    Qt3DExtras::Quick::Qt3DQuickWindow view;
    view.setTitle("Arrows");
    view.resize(1600, 800);
    view.engine()->qmlEngine()->rootContext()->setContextProperty("_window", &view);
    view.setSource(QUrl("qrc:/main.qml"));
    view.show();

    return app.exec();
}
