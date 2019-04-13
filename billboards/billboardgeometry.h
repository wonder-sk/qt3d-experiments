#ifndef BILLBOARDGEOMETRY_H
#define BILLBOARDGEOMETRY_H

#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QBuffer>

#include <QVector3D>


class BillboardGeometry : public Qt3DRender::QGeometry
{
  Q_OBJECT

  Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
  BillboardGeometry( Qt3DCore::QNode *parent = nullptr );

  void setPoints( const QVector<QVector3D> &vertices );

  int count();

signals:
    void countChanged(int count);

private:
  Qt3DRender::QAttribute *mPositionAttribute = nullptr;
  Qt3DRender::QBuffer *mVertexBuffer = nullptr;
  int mVertexCount = 0;
};

#endif // BILLBOARDGEOMETRY_H
