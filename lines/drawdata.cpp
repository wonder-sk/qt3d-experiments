#include "drawdata.h"


#include <Qt3DRender/QAttribute>

LineMeshGeometry::LineMeshGeometry( Qt3DCore::QNode *parent )
  : Qt3DRender::QGeometry( parent )
  , mPositionAttribute( new Qt3DRender::QAttribute( this ) )
  , mIndexAttribute( new Qt3DRender::QAttribute( this ) )
  , mVertexBuffer( new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this ) )
  , mIndexBuffer( new Qt3DRender::QBuffer( Qt3DRender::QBuffer::IndexBuffer, this ) )
{
  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );

  mIndexAttribute->setAttributeType( Qt3DRender::QAttribute::IndexAttribute );
  mIndexAttribute->setVertexBaseType( Qt3DRender::QAttribute::UnsignedInt );

  addAttribute( mPositionAttribute );
  addAttribute( mIndexAttribute );
}

int LineMeshGeometry::vertexCount()
{
  return mIndexCount;
}

void LineMeshGeometry::setVertices( const QVector<QVector3D> &vertices, const QVector<int> &indices )
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


  mIndexCount = indices.count();
  QByteArray indexBufferData;
  indexBufferData.resize( mIndexCount * sizeof(int) );
  int *rawIndexArray = reinterpret_cast<int *>( indexBufferData.data() );
  for (int i = 0; i < indices.count(); ++i)
    rawIndexArray[i] = indices[i];
  mIndexBuffer->setData( indexBufferData );

  mIndexAttribute->setBuffer( mIndexBuffer );
  mIndexAttribute->setCount( mIndexCount );

  emit countChanged(mVertexCount);
}
