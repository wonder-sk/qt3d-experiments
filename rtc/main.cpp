/**
 * This code demonstrates the "relative to center" rendering technique with Qt3D.
 * It is useful when working with large coordinates (e.g. when creating a virtual
 * globe) without getting numerical issues (which would make objects jump around
 * when zoomed in). The idea is that we make sure to do model-view-projection (MVP)
 * matrix calculation in double precision, and only then convert it to single
 * precision floats that will be used on the GPU.
 * 
 * How it is done with Qt3D:
 * - we have our own transform implementation (MyTransform) with double coordinates
 * - we have our own camera implementation (MyCamera) with double coordinates
 * - all our materials have a "my_mvp" uniform, that contains MVP matrix we have
 *   calculated ourselves using MyTransform and MyCamera
 * - on any change of camera's transform or model's transform, we need to update
 *   my_mvp uniform in the materials accordingly
 * 
 * The test scene is just a sphere and two planes partially intersecting the sphere,
 * all moved far away from the scene's origin (see "megaOffset" variable) to
 * demonstrate the issue with large coordinates, and how that creates jitter
 * (especially when the animation is started by pressing SPACE)
 * 
 * Check out rtc.py in this directory if you are just after the math bits.
 * 
 * For more see:
 * https://virtualglobebook.com/
 * https://help.agi.com/AGIComponents/html/BlogPrecisionsPrecisions.htm
 */

#include <math.h>

#include <QApplication>
#include <QKeyEvent>
#include <QTimer>
#include <QColor>
#include <QUrl>
#include <QMatrix4x4>

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCamera>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QGraphicsApiFilter>
#include <QViewport>
#include <QRenderSurfaceSelector>
#include <QCameraSelector>
#include <QDebugOverlay>
#include <QFilterKey>

#include "vector3d.h"
#include "matrix4x4.h"


// comment out to see how things would behave with single precision math
#define USE_DOUBLE


QTimer *timer;
int counter = 0;


Qt3DRender::QMaterial* basicMaterial( QColor color, Qt3DRender::QParameter **pParamMvp )
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

    *pParamMvp = new Qt3DRender::QParameter( QStringLiteral( "my_mvp" ), QMatrix4x4() );
    technique->addParameter( *pParamMvp );

    Qt3DRender::QEffect* effect = new Qt3DRender::QEffect();
    effect->addTechnique(technique);
    material->setEffect(effect);
    return material;
}



Matrix4x4 floatToDoubleMatrix( QMatrix4x4 m )
{
    Matrix4x4 out;
    double *outData = out.data();
    const float *mData = m.constData();
    for ( int i = 0; i < 16; ++i )
        outData[i] = mData[i];  // conversion float->double
    return out;
}


QMatrix4x4 doubleToFloatMatrix( Matrix4x4 m )
{
    QMatrix4x4 out;
    float *outData = out.data();
    const double *mData = m.constData();
    for ( int i = 0; i < 16; ++i )
        outData[i] = mData[i];  // conversion double->float
    return out;
}



// framegraph like from QForwardRenderer but without QCameraSelector
Qt3DRender::QFrameGraphNode * myFrameGraph()
{
    Qt3DRender::QTechniqueFilter *techniqueFilter = new Qt3DRender::QTechniqueFilter();
    Qt3DRender::QFilterKey *forwardRenderingStyle = new Qt3DRender::QFilterKey(techniqueFilter);
    forwardRenderingStyle->setName(QStringLiteral("renderingStyle"));
    forwardRenderingStyle->setValue(QStringLiteral("forward"));
    techniqueFilter->addMatch(forwardRenderingStyle);

    Qt3DRender::QRenderSurfaceSelector *surfaceSelector = new Qt3DRender::QRenderSurfaceSelector( techniqueFilter );

    Qt3DRender::QViewport *viewport = new Qt3DRender::QViewport( surfaceSelector );
    viewport->setNormalizedRect(QRectF(0.0, 0.0, 1.0, 1.0));

    Qt3DRender::QClearBuffers *clearBuffer = new Qt3DRender::QClearBuffers( viewport );
    clearBuffer->setClearColor(Qt::gray);
    clearBuffer->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);

    //Qt3DRender::QDebugOverlay *debugOverlay = new Qt3DRender::QDebugOverlay( clearBuffer );

    return techniqueFilter;
}



// Our double precision 4x4 transform (replaces QTransform from Qt3D)
class MyTransform : public Qt3DCore::QComponent {

  public:
    void setTranslation( Vector3D t ) {
        m_matrix4x4.translate( t );
    }
    
    Matrix4x4 matrix() const { return m_matrix4x4; }

  private:
    Matrix4x4 m_matrix4x4;
};


// Our camera that replaces Qt3D's QCamera (does not even need to be a QEntity as a part of the scene)
class MyCamera
{
  public:
    MyCamera() {
        updateProjectionMatrix();
    }

    void updateProjectionMatrix() {

        const float radians = (m_fovAngleVertical / 2.0f) * 3.14159 / 180;
        const float sine = std::sin(radians);
        if (sine == 0.0f)
            return;
        float cotan = std::cos(radians) / sine;
        float clip = m_far - m_near;

        m_projectionMatrix = Matrix4x4(
            cotan / m_aspectRatio, 0, 0, 0,
            0, cotan, 0, 0,
            0, 0, -(m_near + m_far) / clip, -(2.0f * m_near * m_far) / clip,
            0, 0, -1, 0
            );
    }

    void setDouble( Vector3D position, Vector3D viewCenter ) {

        Vector3D upVector(0.0, 0.0, -1.0); // we're looking at (X,Z) plane
        //Vector3D upVector(0.0, 1.0, 0.0);  // default used in QCamera

        // TODO: do we need any of this???
        /*
        const Vector3D viewDirection = (viewCenter - position).normalized();

        Matrix4x4 transformMatrix;
        transformMatrix.translate(position);

        // default: m_upVector(0.0f, 1.0f, 0.0f)

        // Negative viewDirection because OpenGL convention is looking down -Z
        transformMatrix.rotate(QQuaternion::fromDirection(-viewDirection, m_upVector.normalized()));

        m_transform->setMatrix(transformMatrix);
        */

        // this pretty much figures our rotation from the vectors + applies negative translation of "position"
        Matrix4x4 viewMatrix;
        viewMatrix.lookAt(position, viewCenter, upVector);
        m_viewMatrix4x4 = viewMatrix;
    }

    void setAspectRatio(float aspectRatio) {
        m_aspectRatio = aspectRatio;
        updateProjectionMatrix();
    }

    Matrix4x4 projectionMatrix() const {
        return m_projectionMatrix;
    }

    Matrix4x4 viewMatrix() const {
        return m_viewMatrix4x4;
    }

    float m_near = 0.1;
    float m_far = 1024;
    float m_fovAngleVertical = 25;  // in degrees
    float m_aspectRatio = 1.33333;

    Matrix4x4 m_projectionMatrix;
    Matrix4x4 m_viewMatrix4x4;
};



void updateMvp( MyCamera *camera, MyTransform *transform, Qt3DRender::QParameter *paramMvp )
{
#ifdef USE_DOUBLE

    // GOOD: using doubles

    Matrix4x4 P = camera->projectionMatrix();
    Matrix4x4 V = camera->viewMatrix();
    Matrix4x4 M = transform->matrix();
    qDebug() << "P" << P;
    qDebug() << "M" << M;
    qDebug() << "V" << V;

    // approach A: simpler (in my opinion)
    Matrix4x4 MV = V * M;
    qDebug() << "MV" << MV;
    Matrix4x4 MVP = P * MV;
    paramMvp->setValue( doubleToFloatMatrix( MVP ) );

    // approach B: as described in the virtual globes book
    //Vector3D centerWgs = Vector3D(1089205, 932789, 2009853); // TODO - center of this entity - could be our megaOffset
    //Vector3D centerEye = MV.map(centerWgs);
    //Matrix4x4 MV_RTC = MV;
    //MV_RTC.data()[12] = centerEye.x();
    //MV_RTC.data()[13] = centerEye.y();
    //MV_RTC.data()[14] = centerEye.z();
    //qDebug() << "MV_RTC" << MV_RTC;
    //Matrix4x4 MVP_RTC = P * MV_RTC;
    //paramMvp->setValue( doubleToFloatMatrix( MVP_RTC ) );

    // verification that both approaches A and B give the same result
    //Vector3D testPt(1089205, 932785, 2009853);
    //qDebug() << "final mvp pt:" << MV.map(testPt);
    //qDebug() << "final rtc pt:" << MV_RTC.map(testPt);

#else

    // BAD: using floating point arithmetics

    QMatrix4x4 P = doubleToFloatMatrix( camera->projectionMatrix() );
    QMatrix4x4 V = doubleToFloatMatrix( camera->viewMatrix() );
    QMatrix4x4 M = doubleToFloatMatrix( transform->matrix() );
    QMatrix4x4 MV = V * M;
    QMatrix4x4 MVP = P * MV;
    qDebug() << "MVP" << MVP;
    paramMvp->setValue( MVP );
#endif
}


// updates MVP matrix in the material of all entities
void updateAllMvp( MyCamera *camera, QList< QPair< MyTransform*, Qt3DRender::QParameter *> > lst )
{
    for ( auto pair : lst )
    {
        updateMvp(camera, pair.first, pair.second);
    }
}



// start/stop animation with a space
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



int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Create the 3D window
    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();

    timer = new QTimer(view);
    timer->setInterval(20);

    MyEventFilter *myEventFilter = new MyEventFilter(view);
    view->installEventFilter(myEventFilter);

    // geometries

    Qt3DExtras::QPlaneMesh *planeMeshA = new Qt3DExtras::QPlaneMesh;

    Qt3DExtras::QPlaneMesh *planeMeshB = new Qt3DExtras::QPlaneMesh;

    Qt3DExtras::QSphereMesh *sphereMesh = new Qt3DExtras::QSphereMesh;
    sphereMesh->setRadius(2.0f);

    // materials

    Qt3DRender::QParameter *paramMvpRed;
    Qt3DRender::QParameter *paramMvpGreen;
    Qt3DRender::QParameter *paramMvpBlue;
    Qt3DRender::QMaterial *materialRed = basicMaterial(Qt::red, &paramMvpRed);
    Qt3DRender::QMaterial *materialGreen = basicMaterial(Qt::green, &paramMvpGreen);
    Qt3DRender::QMaterial *materialBlue = basicMaterial(Qt::blue, &paramMvpBlue);

    // transforms

    //Vector3D megaOffset(0, 0, 0);
    //Vector3D megaOffset(10, 10, 10);
    //Vector3D megaOffset(10000, 10000, 10000);
    Vector3D megaOffset(1089205, 932789, 2009853);

    float animationRange = 0.1;

    MyTransform *sphereTransform = new MyTransform;
    sphereTransform->setTranslation(Vector3D(0.0, 0.0, 0.0) + megaOffset);

    MyTransform *planeATransform = new MyTransform;
    planeATransform->setTranslation(Vector3D(-0.51, 1.98, 0.0) + megaOffset);

    MyTransform *planeBTransform = new MyTransform;
    planeBTransform->setTranslation(Vector3D(+0.51, 1.98, 0.0) + megaOffset);

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

    QList< QPair< MyTransform*, Qt3DRender::QParameter *> > allEntities;
    allEntities << qMakePair(sphereTransform, paramMvpRed);
    allEntities << qMakePair(planeATransform, paramMvpGreen);
    allEntities << qMakePair(planeBTransform, paramMvpBlue);

    // set up frame graph
    // (like a frame graph from QForwardRenderer, but without frustum culling + camera selector)

    view->setActiveFrameGraph( myFrameGraph() );

    // set up our camera (that has view matrix in double coordinates)

    MyCamera *myCamera = new MyCamera;
    //myCamera->setParent(rootEntity);
    myCamera->setAspectRatio(float(view->width()) / std::max(1.f, static_cast<float>(view->height())));

    QObject::connect( view, &QWindow::widthChanged, [view, myCamera, allEntities] {
        myCamera->setAspectRatio(float(view->width()) / std::max(1.f, static_cast<float>(view->height())));
        updateAllMvp(myCamera, allEntities);
    });
    QObject::connect( view, &QWindow::heightChanged, [view, myCamera, allEntities] {
        myCamera->setAspectRatio(float(view->width()) / std::max(1.f, static_cast<float>(view->height())));
        updateAllMvp(myCamera, allEntities);
    });

    // Setup camera
    Vector3D cameraBasePosition(1.0, 10.0, 0.0);
    Vector3D cameraBaseViewCenter(0.0, 0.0, 0.0);
    cameraBasePosition = cameraBasePosition + megaOffset;
    cameraBaseViewCenter = cameraBaseViewCenter + megaOffset;
    myCamera->setDouble( cameraBasePosition, cameraBaseViewCenter );

    updateAllMvp(myCamera, allEntities);

    // Add root entity to scene
    view->setRootEntity(rootEntity);

    view->show();

    QObject::connect( timer, &QTimer::timeout, [myCamera, allEntities, animationRange, cameraBasePosition, cameraBaseViewCenter] {
        ++counter;
        double t = (double) (counter % 50) / 50.;
        Vector3D tOffset(sin(t*3.14159*2) * animationRange, 0, cos(t*3.14159*2) * animationRange);

        myCamera->setDouble( cameraBasePosition + tOffset, cameraBaseViewCenter + tOffset );

        updateAllMvp(myCamera, allEntities);
    });

    return app.exec();
}
