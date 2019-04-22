#include <Qt3DQuickExtras/qt3dquickwindow.h>
#include <Qt3DQuick/QQmlAspectEngine>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>

#include "instancedgeometry.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QVector<QVector3D> pos;
    pos << QVector3D(1, 1, 0);
    pos << QVector3D(-1, 2, 8);
    pos << QVector3D(1, 1, 7);
    pos << QVector3D(0, 0, 4);
    pos << QVector3D(1, 5, 1);
    pos << QVector3D(-3, 3, 0);
    pos << QVector3D(2, 2, -2);

    InstancedGeometry instGeom;
    instGeom.setPoints(pos);

    Qt3DExtras::Quick::Qt3DQuickWindow view;
    view.setTitle("Instanced Rendering");
    view.resize(1600, 800);
    view.engine()->qmlEngine()->rootContext()->setContextProperty("_window", &view);
    view.engine()->qmlEngine()->rootContext()->setContextProperty("_instg", &instGeom);
    view.setSource(QUrl("qrc:/main.qml"));
    view.show();

    return app.exec();
}
