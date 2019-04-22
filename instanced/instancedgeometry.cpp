#include "instancedgeometry.h"

#include <Qt3DRender/QAttribute>


InstancedGeometry::InstancedGeometry( Qt3DCore::QNode *parent )
  : Qt3DExtras::QSphereGeometry( parent )
  , mPositionAttribute( new Qt3DRender::QAttribute( this ) )
  , mInstanceBuffer( new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this ) )
{

  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mInstanceBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setName( QStringLiteral( "pos" ) );
  mPositionAttribute->setDivisor( 1 );
  mPositionAttribute->setByteStride( 3 * sizeof( float ) );

  addAttribute( mPositionAttribute );
  setBoundingVolumePositionAttribute( mPositionAttribute );
}

int InstancedGeometry::count()
{
  return mInstanceCount;
}

QMatrix4x4 InstancedGeometry::normalMatrix(QMatrix4x4 mat)
{
  QMatrix3x3 normalMatrix = mat.normalMatrix();

  // QMatrix3x3 is not supported for passing to shaders, so we pass QMatrix4x4
  float *n = normalMatrix.data();
  QMatrix4x4 normalMatrix4(
    n[0], n[3], n[6], 0,
    n[1], n[4], n[7], 0,
    n[2], n[5], n[8], 0,
    0, 0, 0, 0 );

  return normalMatrix4;
}

void InstancedGeometry::setPoints(const QVector<QVector3D> &vertices)
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( vertices.size() * 3 * sizeof( float ) );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( const auto &v : vertices )
  {
    rawVertexArray[idx++] = v.x();
    rawVertexArray[idx++] = v.y();
    rawVertexArray[idx++] = v.z();
  }

  mInstanceCount = vertices.count();
  mInstanceBuffer->setData( vertexBufferData );

  mPositionAttribute->setCount( mInstanceCount );

  emit countChanged(mInstanceCount);
}
