#ifndef INSTANCEDGEOMETRY_H
#define INSTANCEDGEOMETRY_H

#include <Qt3DExtras/QSphereGeometry>
#include <Qt3DRender/QBuffer>

#include <QVector3D>

#include <QMatrix4x4>

class InstancedGeometry : public Qt3DExtras::QSphereGeometry
{
  Q_OBJECT

  Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
  InstancedGeometry( Qt3DCore::QNode *parent = nullptr );

  void setPoints( const QVector<QVector3D> &vertices );

  int count();

  Q_INVOKABLE static QMatrix4x4 normalMatrix(QMatrix4x4 mat);

signals:
    void countChanged(int count);

private:
  Qt3DRender::QAttribute *mPositionAttribute = nullptr;
  Qt3DRender::QBuffer *mInstanceBuffer = nullptr;
  int mInstanceCount = 0;
};

#endif // INSTANCEDGEOMETRY_H
