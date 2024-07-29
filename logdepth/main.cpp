/**
 * This code demonstrates logarithmic depth buffer rendering technique.
 * It is a way to increase the precision of the depth buffer when using
 * large depth range (e.g. when creating a virtual globe). The idea is
 * that the fragment shader sets depth of fragments to make a better
 * use of the range [0..1] instead of keeping the depth value that came
 * out from the projection matrix.
 * 
 *   frag_depth = log(1 + depth) / log(1 + far_plane_depth)
 * 
 * Nowadays, the issue with depth buffer precision is most commonly handled
 * with "Reverse Z" technique, which does not need any changes in shaders,
 * but unfortunately at this point, it can't be used in Qt3D with OpenGL
 * renderer, because that technique needs a call to glClipControl(), which
 * is currently not supported by Qt3D.
 * 
 * The test scene is just a sphere and two planes partially intersecting the sphere,
 * with near and far planes set so that the depth buffer precision issue becomes
 * clearly visible. To see how things would behave without logarithmic depth buffer,
 * just comment out the line with "gl_FragDepth" in basic.frag.
 * 
 * For more see:
 * https://virtualglobebook.com/
 * https://outerra.blogspot.com/2013/07/logarithmic-depth-buffer-optimizations.html
 */

#include <QApplication>

#include <Qt3DExtras/Qt3DWindow>

#include <QTimer>
#include <QColor>
#include <QUrl>
#include <QMatrix4x4>
#include <QKeyEvent>

#include <Qt3DRender/QCamera>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DCore/QTransform>


#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <math.h>


QTimer *timer;
int counter = 0;


class MyEventFilter : public QObject {

public:
    explicit MyEventFilter(QObject* parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Space) {

                qDebug() << "SPACE!";
                if ( timer->isActive() ) timer->stop(); else timer->start();

                // You can optionally consume the event here
                // event->accept();
                return true;
            }
        }
        // Pass the event to the parent or base class if not handled
        return QObject::eventFilter(watched, event);
    }
};




Qt3DRender::QMaterial* basicMaterial( QColor color )
{
    Qt3DRender::QMaterial *material = new Qt3DRender::QMaterial();

    Qt3DRender::QShaderProgram* shaderProgram = new Qt3DRender::QShaderProgram();
    shaderProgram->setVertexShaderCode(Qt3DRender::QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/basic.vert"))));
    shaderProgram->setFragmentShaderCode(Qt3DRender::QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/basic.frag"))));

    Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass();
    renderPass->setShaderProgram(shaderProgram);

    Qt3DRender::QTechnique* technique = new Qt3DRender::QTechnique();
    technique->addRenderPass(renderPass);

    technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
    technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
    technique->graphicsApiFilter()->setMajorVersion( 3 );
    technique->graphicsApiFilter()->setMinorVersion( 3 );
    Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey();
    filterKey->setName( QStringLiteral( "renderingStyle" ) );
    filterKey->setValue( QStringLiteral( "forward" ) );
    technique->addFilterKey( filterKey );

    technique->addParameter( new Qt3DRender::QParameter( QStringLiteral( "color" ), color ) );

    technique->addParameter( new Qt3DRender::QParameter( QStringLiteral( "farPlane" ), 1000000.0 ) );

    Qt3DRender::QEffect* effect = new Qt3DRender::QEffect();
    effect->addTechnique(technique);
    material->setEffect(effect);
    return material;
}



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Create the 3D window
    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();

    // geometries

    Qt3DExtras::QPlaneMesh *planeMeshA = new Qt3DExtras::QPlaneMesh;

    Qt3DExtras::QPlaneMesh *planeMeshB = new Qt3DExtras::QPlaneMesh;

    Qt3DExtras::QSphereMesh *sphereMesh = new Qt3DExtras::QSphereMesh;
    sphereMesh->setRadius(2.0f);

    // materials

    Qt3DRender::QMaterial *materialRed = basicMaterial(Qt::red);
    Qt3DRender::QMaterial *materialGreen = basicMaterial(Qt::green);
    Qt3DRender::QMaterial *materialBlue = basicMaterial(Qt::blue);

    // transforms

    Qt3DCore::QTransform *sphereTransform = new Qt3DCore::QTransform;
    sphereTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));

    Qt3DCore::QTransform *planeATransform = new Qt3DCore::QTransform;
    planeATransform->setTranslation(QVector3D(-0.51f, 1.98f, 0.0f));

    Qt3DCore::QTransform *planeBTransform = new Qt3DCore::QTransform;
    planeBTransform->setTranslation(QVector3D(+0.51f, 1.98f, 0.0f));

    // scene

    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity;

    Qt3DCore::QEntity *sphereEntity = new Qt3DCore::QEntity(rootEntity);
    sphereEntity->addComponent(sphereMesh);
    sphereEntity->addComponent(materialRed);
    sphereEntity->addComponent(sphereTransform);

    Qt3DCore::QEntity *planeAEntity = new Qt3DCore::QEntity(rootEntity);
    planeAEntity->addComponent(planeMeshA);
    planeAEntity->addComponent(materialGreen);
    planeAEntity->addComponent(planeATransform);

    Qt3DCore::QEntity *planeBEntity = new Qt3DCore::QEntity(rootEntity);
    planeBEntity->addComponent(planeMeshB);
    planeBEntity->addComponent(materialBlue);
    planeBEntity->addComponent(planeBTransform);

    //

    Qt3DExtras::QForwardRenderer *forwardRenderer = view->defaultFrameGraph();

    QVector3D cameraBasePosition(1.0, 10.0, 0.0);
    QVector3D cameraBaseViewCenter(0.0, 0.0, 0.0);
    Qt3DRender::QCamera *camera = qobject_cast<Qt3DRender::QCamera*>( forwardRenderer->camera() );
    camera->setPosition( cameraBasePosition );
    camera->setViewCenter( cameraBaseViewCenter );

    camera->setNearPlane(0.001);
    camera->setFarPlane(1000000);

    qDebug() << "near/far" << camera->nearPlane() << camera->farPlane();

    // Add root entity to scene
    view->setRootEntity(rootEntity);

    view->show();

    timer = new QTimer(view);
    timer->setInterval(20);

    MyEventFilter *myEventFilter = new MyEventFilter(view);
    view->installEventFilter(myEventFilter);

    float animationRange = 0.1;

    QObject::connect( timer, &QTimer::timeout, [camera, animationRange, cameraBasePosition, cameraBaseViewCenter] {
        ++counter;
        double t = (double) (counter % 50) / 50.;
        QVector3D tOffset(sin(t*3.14159*2) * animationRange, 0, cos(t*3.14159*2) * animationRange);

        camera->setPosition( cameraBasePosition + tOffset );
        camera->setViewCenter( cameraBaseViewCenter + tOffset );

    });


    return a.exec();
}
