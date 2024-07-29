
#include "matrix4x4.h"

// the implementation is partially based on Qt's QMatrix4x4 (simplified)


Matrix4x4::Matrix4x4( double m11, double m12, double m13, double m14,
                      double m21, double m22, double m23, double m24,
                      double m31, double m32, double m33, double m34,
                      double m41, double m42, double m43, double m44 )
{
  m[0][0] = m11; m[0][1] = m21; m[0][2] = m31; m[0][3] = m41;
  m[1][0] = m12; m[1][1] = m22; m[1][2] = m32; m[1][3] = m42;
  m[2][0] = m13; m[2][1] = m23; m[2][2] = m33; m[2][3] = m43;
  m[3][0] = m14; m[3][1] = m24; m[3][2] = m34; m[3][3] = m44;
}

void Matrix4x4::translate( const Vector3D &vector )
{
  m[3][0] += m[0][0] * vector.x() + m[1][0] * vector.y() + m[2][0] * vector.z();
  m[3][1] += m[0][1] * vector.x() + m[1][1] * vector.y() + m[2][1] * vector.z();
  m[3][2] += m[0][2] * vector.x() + m[1][2] * vector.y() + m[2][2] * vector.z();
  m[3][3] += m[0][3] * vector.x() + m[1][3] * vector.y() + m[2][3] * vector.z();
}

#if 0
QList< double > Matrix4x4::dataList() const
{
  QList< double > res;
  res.reserve( 9 );
  for ( int i = 0; i < 16; ++i )
  {
    res.append( m[i / 4][i % 4] );
  }
  return res;
}
#endif

Vector3D operator*( const Matrix4x4 &matrix, const Vector3D &vector )
{
  double x, y, z, w;

  x = vector.x() * matrix.m[0][0] +
      vector.y() * matrix.m[1][0] +
      vector.z() * matrix.m[2][0] +
      matrix.m[3][0];
  y = vector.x() * matrix.m[0][1] +
      vector.y() * matrix.m[1][1] +
      vector.z() * matrix.m[2][1] +
      matrix.m[3][1];
  z = vector.x() * matrix.m[0][2] +
      vector.y() * matrix.m[1][2] +
      vector.z() * matrix.m[2][2] +
      matrix.m[3][2];
  w = vector.x() * matrix.m[0][3] +
      vector.y() * matrix.m[1][3] +
      vector.z() * matrix.m[2][3] +
      matrix.m[3][3];
  if ( w == 1.0f )
    return Vector3D( x, y, z );
  else
    return Vector3D( x / w, y / w, z / w );
}

bool Matrix4x4::isIdentity() const
{
  if ( m[0][0] != 1.0 || m[0][1] != 0.0 || m[0][2] != 0.0 )
    return false;
  if ( m[0][3] != 0.0 || m[1][0] != 0.0 || m[1][1] != 1.0 )
    return false;
  if ( m[1][2] != 0.0 || m[1][3] != 0.0 || m[2][0] != 0.0 )
    return false;
  if ( m[2][1] != 0.0 || m[2][2] != 1.0 || m[2][3] != 0.0 )
    return false;
  if ( m[3][0] != 0.0 || m[3][1] != 0.0 || m[3][2] != 0.0 )
    return false;
  return ( m[3][3] == 1.0 );
}

void Matrix4x4::setToIdentity()
{
  m[0][0] = 1.0;
  m[0][1] = 0.0;
  m[0][2] = 0.0;
  m[0][3] = 0.0;
  m[1][0] = 0.0;
  m[1][1] = 1.0;
  m[1][2] = 0.0;
  m[1][3] = 0.0;
  m[2][0] = 0.0;
  m[2][1] = 0.0;
  m[2][2] = 1.0;
  m[2][3] = 0.0;
  m[3][0] = 0.0;
  m[3][1] = 0.0;
  m[3][2] = 0.0;
  m[3][3] = 1.0;
}


Matrix4x4 operator*( const Matrix4x4 &m1, const Matrix4x4 &m2 )
{
  Matrix4x4 m( 1 );
  m.m[0][0] = m1.m[0][0] * m2.m[0][0]
              + m1.m[1][0] * m2.m[0][1]
              + m1.m[2][0] * m2.m[0][2]
              + m1.m[3][0] * m2.m[0][3];
  m.m[0][1] = m1.m[0][1] * m2.m[0][0]
              + m1.m[1][1] * m2.m[0][1]
              + m1.m[2][1] * m2.m[0][2]
              + m1.m[3][1] * m2.m[0][3];
  m.m[0][2] = m1.m[0][2] * m2.m[0][0]
              + m1.m[1][2] * m2.m[0][1]
              + m1.m[2][2] * m2.m[0][2]
              + m1.m[3][2] * m2.m[0][3];
  m.m[0][3] = m1.m[0][3] * m2.m[0][0]
              + m1.m[1][3] * m2.m[0][1]
              + m1.m[2][3] * m2.m[0][2]
              + m1.m[3][3] * m2.m[0][3];

  m.m[1][0] = m1.m[0][0] * m2.m[1][0]
              + m1.m[1][0] * m2.m[1][1]
              + m1.m[2][0] * m2.m[1][2]
              + m1.m[3][0] * m2.m[1][3];
  m.m[1][1] = m1.m[0][1] * m2.m[1][0]
              + m1.m[1][1] * m2.m[1][1]
              + m1.m[2][1] * m2.m[1][2]
              + m1.m[3][1] * m2.m[1][3];
  m.m[1][2] = m1.m[0][2] * m2.m[1][0]
              + m1.m[1][2] * m2.m[1][1]
              + m1.m[2][2] * m2.m[1][2]
              + m1.m[3][2] * m2.m[1][3];
  m.m[1][3] = m1.m[0][3] * m2.m[1][0]
              + m1.m[1][3] * m2.m[1][1]
              + m1.m[2][3] * m2.m[1][2]
              + m1.m[3][3] * m2.m[1][3];

  m.m[2][0] = m1.m[0][0] * m2.m[2][0]
              + m1.m[1][0] * m2.m[2][1]
              + m1.m[2][0] * m2.m[2][2]
              + m1.m[3][0] * m2.m[2][3];
  m.m[2][1] = m1.m[0][1] * m2.m[2][0]
              + m1.m[1][1] * m2.m[2][1]
              + m1.m[2][1] * m2.m[2][2]
              + m1.m[3][1] * m2.m[2][3];
  m.m[2][2] = m1.m[0][2] * m2.m[2][0]
              + m1.m[1][2] * m2.m[2][1]
              + m1.m[2][2] * m2.m[2][2]
              + m1.m[3][2] * m2.m[2][3];
  m.m[2][3] = m1.m[0][3] * m2.m[2][0]
              + m1.m[1][3] * m2.m[2][1]
              + m1.m[2][3] * m2.m[2][2]
              + m1.m[3][3] * m2.m[2][3];

  m.m[3][0] = m1.m[0][0] * m2.m[3][0]
              + m1.m[1][0] * m2.m[3][1]
              + m1.m[2][0] * m2.m[3][2]
              + m1.m[3][0] * m2.m[3][3];
  m.m[3][1] = m1.m[0][1] * m2.m[3][0]
              + m1.m[1][1] * m2.m[3][1]
              + m1.m[2][1] * m2.m[3][2]
              + m1.m[3][1] * m2.m[3][3];
  m.m[3][2] = m1.m[0][2] * m2.m[3][0]
              + m1.m[1][2] * m2.m[3][1]
              + m1.m[2][2] * m2.m[3][2]
              + m1.m[3][2] * m2.m[3][3];
  m.m[3][3] = m1.m[0][3] * m2.m[3][0]
              + m1.m[1][3] * m2.m[3][1]
              + m1.m[2][3] * m2.m[3][2]
              + m1.m[3][3] * m2.m[3][3];
  return m;
}


/*!
    Multiplies this matrix by a viewing matrix derived from an eye
    point. The \a center value indicates the center of the view that
    the \a eye is looking at.  The \a up value indicates which direction
    should be considered up with respect to the \a eye.

    \note The \a up vector must not be parallel to the line of sight
    from \a eye to \a center.
*/
void Matrix4x4::lookAt(const Vector3D& eye, const Vector3D& center, const Vector3D& up)
{
  Vector3D forward = center - eye;
  if (qFuzzyIsNull(forward.x()) && qFuzzyIsNull(forward.y()) && qFuzzyIsNull(forward.z()))
    return;

  forward.normalize();
  Vector3D side = Vector3D::crossProduct(forward, up).normalized();
  Vector3D upVector = Vector3D::crossProduct(side, forward);

  Matrix4x4 m; //(Qt::Uninitialized);
  m.m[0][0] = side.x();
  m.m[1][0] = side.y();
  m.m[2][0] = side.z();
  m.m[3][0] = 0.0f;
  m.m[0][1] = upVector.x();
  m.m[1][1] = upVector.y();
  m.m[2][1] = upVector.z();
  m.m[3][1] = 0.0f;
  m.m[0][2] = -forward.x();
  m.m[1][2] = -forward.y();
  m.m[2][2] = -forward.z();
  m.m[3][2] = 0.0f;
  m.m[0][3] = 0.0f;
  m.m[1][3] = 0.0f;
  m.m[2][3] = 0.0f;
  m.m[3][3] = 1.0f;

  if ( !isIdentity() )
    *this = *this * m;
  else
    *this = m;
  translate(-eye);
}
