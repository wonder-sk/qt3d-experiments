#include <Qt3DQuickExtras/qt3dquickwindow.h>
#include <Qt3DQuick/QQmlAspectEngine>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>

#include "drawdata.h"


int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QVector3D px(0,-1,0);  // used as restart primitive (index zero)

    QVector3D p1(0,0,0);
    QVector3D p2(0,5,0);
    QVector3D p3(-5, 5, -5);
    QVector3D p4(5, 5, 5);
    QVector3D p5(-5, 5, 5);
    QVector3D p6(5, 5, -5);
    QVector3D p7(5, 1, -5);

    QVector<QVector3D> pos;
    QVector<int> indices;

    pos << px << p1 << p2 << p3 << p4 << p5 << p6 << p7;

    indices << 1 << 1 << 2 << 2 << 0;
    indices << 3 << 3 << 4 << 4 << 0;
    indices << 5 << 5 << 6 << 7 << 7 << 0;

    // make a cube for testing
    int i0 = 8;
    QVector3D o(2,2,5);
    pos << QVector3D(-1,-1,-1)+o << QVector3D(+1,-1,-1)+o << QVector3D(+1,+1,-1)+o << QVector3D(-1,+1,-1)+o;
    pos << QVector3D(-1,-1,+1)+o << QVector3D(+1,-1,+1)+o << QVector3D(+1,+1,+1)+o << QVector3D(-1,+1,+1)+o;
    indices << i0+3 << i0+0 << i0+1 << i0+2 << i0+3 << i0+0 << i0+1 << 0;
    indices << i0+7 << i0+4 << i0+5 << i0+6 << i0+7 << i0+4 << i0+5 << 0;
    indices << i0+3 << i0+0 << i0+4 << i0+7 << i0+3 << i0+0 << i0+4 << 0;
    indices << i0+2 << i0+1 << i0+5 << i0+6 << i0+2 << i0+1 << i0+5 << 0;

    // lines adjacency primitive: each line is given as (prev)-(p0)-(p1)-(next)

    LineMeshGeometry lmg;
    lmg.setVertices(pos, indices);

    Qt3DExtras::Quick::Qt3DQuickWindow view;
    view.setTitle("Lines");
    view.resize(1600, 800);
    view.engine()->qmlEngine()->rootContext()->setContextProperty("_window", &view);
    view.engine()->qmlEngine()->rootContext()->setContextProperty("_lmg", &lmg);
    view.setSource(QUrl("qrc:/main.qml"));
    view.show();

    return app.exec();
}
