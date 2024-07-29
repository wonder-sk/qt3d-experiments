
#ifndef MATRIX4X4_H
#define MATRIX4X4_H


#include "vector3d.h"

class Matrix4x4
{
  public:
    //! Initializes identity matrix
    Matrix4x4() { setToIdentity(); }
    //! Initializes matrix by setting all values in row-major order
    Matrix4x4( double m11, double m12, double m13, double m14,
               double m21, double m22, double m23, double m24,
               double m31, double m32, double m33, double m34,
               double m41, double m42, double m43, double m44 );

    bool operator==( const Matrix4x4 &other ) const
    {
      const double *data = *m;
      const double *otherData = *( other.m );
      for ( int i = 0; i < 16; ++i, data++, otherData++ )
      {
        if ( !qgsDoubleNear( *data, *otherData ) )
          return false;
      }
      return true;
    }

    bool operator!=( const Matrix4x4 &other ) const
    {
      return !( *this == other );
    }

    //! Returns pointer to the matrix data (stored in column-major order)
    const double *constData() const { return *m; }
    //! Returns pointer to the matrix data (stored in column-major order)
    double *data() { return *m; }
#if 0
    //! Returns matrix data (in column-major order)
    QList< double > dataList() const;
#endif

    /**
     * Multiplies this matrix by another that translates coordinates by the components of a \a vector.
     */
    void translate( const Vector3D &vector );

    //! Matrix-vector multiplication (vector is converted to homogeneous coordinates [X,Y,Z,1] and back)
    Vector3D map( const Vector3D &vector ) const
    {
      return *this * vector;
    }

    //! Returns whether this matrix is an identity matrix
    bool isIdentity() const;
    //! Sets matrix to be identity matrix
    void setToIdentity();

    friend Matrix4x4 operator*( const Matrix4x4 &m1, const Matrix4x4 &m2 );
    friend Vector3D operator*( const Matrix4x4 &matrix, const Vector3D &vector );

    void lookAt(const Vector3D& eye, const Vector3D& center, const Vector3D& up);

    const double& operator()(int aRow, int aColumn) const
    {
      Q_ASSERT(aRow >= 0 && aRow < 4 && aColumn >= 0 && aColumn < 4);
      return m[aColumn][aRow];
    }

  private:
    // Matrix data - in column-major order
    double m[4][4];

    //! Construct without initializing identity matrix.
    explicit Matrix4x4( int ) { }   // cppcheck-suppress uninitMemberVarPrivate
};

//! Matrix-matrix multiplication (useful to concatenate transforms)
Vector3D operator*( const Matrix4x4 &matrix, const Vector3D &vector );
//! Matrix-vector multiplication (vector is converted to homogeneous coordinates [X,Y,Z,1] and back)
Matrix4x4 operator*( const Matrix4x4 &m1, const Matrix4x4 &m2 );


#include <QDebug>

inline QDebug operator<<(QDebug dbg, const Matrix4x4 &m)
{
    QDebugStateSaver saver(dbg);

    // Output in row-major order because it is more human-readable.
    dbg.nospace() << "Matrix4x4(" << Qt::endl
                  << qSetFieldWidth(10)
                  << m(0, 0) << m(0, 1) << m(0, 2) << m(0, 3) << Qt::endl
                  << m(1, 0) << m(1, 1) << m(1, 2) << m(1, 3) << Qt::endl
                  << m(2, 0) << m(2, 1) << m(2, 2) << m(2, 3) << Qt::endl
                  << m(3, 0) << m(3, 1) << m(3, 2) << m(3, 3) << Qt::endl
                  << qSetFieldWidth(0) << ')';
    return dbg;
}

#endif
