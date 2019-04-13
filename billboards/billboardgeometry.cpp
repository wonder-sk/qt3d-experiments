#include "billboardgeometry.h"

#include <Qt3DRender/QAttribute>


BillboardGeometry::BillboardGeometry( Qt3DCore::QNode *parent )
  : Qt3DRender::QGeometry( parent )
  , mPositionAttribute( new Qt3DRender::QAttribute( this ) )
  , mVertexBuffer( new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this ) )
{

  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );

  addAttribute( mPositionAttribute );

}

int BillboardGeometry::count()
{
  return mVertexCount;
}

void BillboardGeometry::setPoints(const QVector<QVector3D> &vertices)
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

  mVertexCount = vertices.count();
  mVertexBuffer->setData( vertexBufferData );

  emit countChanged(mVertexCount);
}
