#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QCommandLineParser>
#include <QFileInfo>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>

#include <Qt3DRender/QCamera>

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QDebugOverlay>

#include "splatrenderer.h"

Qt3DCore::QEntity* createScene(SplatRenderer*& outSplatRenderer)
{
    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity;

    // Create splat renderer entity
    outSplatRenderer = new SplatRenderer(rootEntity);

    return rootEntity;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Qt 3D Gaussian Splatting");
    QApplication::setApplicationVersion("1.0");

    // Parse command line
    QCommandLineParser parser;
    parser.setApplicationDescription("3D Gaussian Splatting Viewer using Qt 3D");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("plyfile", "PLY file containing Gaussian splats");

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        qWarning() << "Usage: qt3d-splat <plyfile.ply>";
        qWarning() << "No PLY file specified. Please provide a Gaussian splatting PLY file.";
        return 1;
    }

    QString plyFile = args.first();
    if (!QFileInfo::exists(plyFile)) {
        qWarning() << "PLY file not found:" << plyFile;
        return 1;
    }

    // Create the 3D window
    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow;
    view->defaultFrameGraph()->setClearColor(QColor(20, 20, 20));

    // Add debug overlay for FPS display
    Qt3DRender::QDebugOverlay *debugOverlay = new Qt3DRender::QDebugOverlay;
    debugOverlay->setParent(view->activeFrameGraph());
    debugOverlay->setEnabled(true);

    // Create a container widget to hold the 3D view
    QWidget *container = QWidget::createWindowContainer(view);
    container->setMinimumSize(1024, 768);
    container->setWindowTitle("Qt 3D Gaussian Splatting - " + QFileInfo(plyFile).fileName());

    // Create the scene
    SplatRenderer *splatRenderer = nullptr;
    Qt3DCore::QEntity *rootEntity = createScene(splatRenderer);

    // Load the PLY file
    qDebug() << "Loading PLY file:" << plyFile;
    if (!splatRenderer->loadPLY(plyFile)) {
        qWarning() << "Failed to load PLY file:" << plyFile;
        return 1;
    }

    // Setup camera based on scene bounds
    Qt3DRender::QCamera *camera = view->camera();
    QVector3D sceneCenter = splatRenderer->sceneCenter();
    float sceneRadius = splatRenderer->sceneRadius();

    // Position camera to see the whole scene
    float cameraDistance = sceneRadius * 2.0f;
    QVector3D cameraPos = sceneCenter + QVector3D(0, 0, cameraDistance);

    qDebug() << "Camera setup: center=" << sceneCenter << "radius=" << sceneRadius
             << "distance=" << cameraDistance << "pos=" << cameraPos;

    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.001f, sceneRadius * 10.0f);
    camera->setPosition(cameraPos);
    camera->setViewCenter(sceneCenter);
    camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));

    // Connect camera to splat renderer
    splatRenderer->setCamera(camera);
    splatRenderer->setViewportSize(container->size());

    // Orbit camera controller
    Qt3DExtras::QOrbitCameraController *cameraController = new Qt3DExtras::QOrbitCameraController(rootEntity);
    cameraController->setCamera(camera);
    cameraController->setLinearSpeed(50.0f);
    cameraController->setLookSpeed(180.0f);

    // Set root entity
    view->setRootEntity(rootEntity);

    // Update viewport size when window resizes
    QObject::connect(container, &QWidget::windowTitleChanged, [&]() {
        splatRenderer->setViewportSize(container->size());
    });

    // Show the window
    container->show();

    // Update viewport after showing
    splatRenderer->setViewportSize(container->size());

    return app.exec();
}
