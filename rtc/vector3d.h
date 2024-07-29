
#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <cmath>

#include <QVector3D>


template<typename T>
inline bool qgsNumberNear( T a, T b, T epsilon = std::numeric_limits<T>::epsilon() * 4 )
{
    const bool aIsNan = std::isnan( a );
    const bool bIsNan = std::isnan( b );
    if ( aIsNan || bIsNan )
        return aIsNan && bIsNan;

    const T diff = a - b;
    return diff >= -epsilon && diff <= epsilon;
}

inline bool qgsDoubleNear( double a, double b, double epsilon = 4 * std::numeric_limits<double>::epsilon() )
{
    return qgsNumberNear<double>( a, b, epsilon );
}

class Vector3D
{
  public:
    //! Constructs a null vector
    Vector3D() = default;

    //! Constructs a vector from given coordinates
    Vector3D( double x, double y, double z )
      : mX( x ), mY( y ), mZ( z ) {}

    //! Constructs a vector from single-precision QVector3D
    Vector3D( const QVector3D &v )
      : mX( v.x() ), mY( v.y() ), mZ( v.z() ) {}

    //! Returns TRUE if all three coordinates are zero
    bool isNull() const { return mX == 0 && mY == 0 && mZ == 0; }

    //! Returns X coordinate
    double x() const { return mX; }
    //! Returns Y coordinate
    double y() const { return mY; }
    //! Returns Z coordinate
    double z() const { return mZ; }

    /**
     * Sets X coordinate
     * \since QGIS 3.34
     */
    void setX( double x ) { mX = x; }

    /**
     * Sets Y coordinate
     * \since QGIS 3.34
     */
    void setY( double y ) { mY = y; }

    /**
     * Sets Z coordinate
     * \since QGIS 3.34
     */
    void setZ( double z ) { mZ = z; }

    //! Sets vector coordinates
    void set( double x, double y, double z )
    {
      mX = x;
      mY = y;
      mZ = z;
    }

    // TODO c++20 - replace with = default
    bool operator==( const Vector3D &other ) const
    {
      return mX == other.mX && mY == other.mY && mZ == other.mZ;
    }
    bool operator!=( const Vector3D &other ) const
    {
      return !operator==( other );
    }

    //! Returns sum of two vectors
    Vector3D operator+( const Vector3D &other ) const
    {
      return Vector3D( mX + other.mX, mY + other.mY, mZ + other.mZ );
    }

    //! Returns difference of two vectors
    Vector3D operator-( const Vector3D &other ) const
    {
      return Vector3D( mX - other.mX, mY - other.mY, mZ - other.mZ );
    }

    //! Returns a new vector multiplied by scalar
    Vector3D operator *( const double factor ) const
    {

      return Vector3D( mX * factor, mY * factor, mZ * factor );
    }

    //! Returns a new vector divided by scalar
    Vector3D operator /( const double factor ) const
    {
      return Vector3D( mX / factor, mY / factor, mZ / factor );
    }

    //! Returns the dot product of two vectors
    static double dotProduct( const Vector3D &v1, const Vector3D &v2 )
    {
      return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
    }

    //! Returns the cross product of two vectors
    static Vector3D crossProduct( const Vector3D &v1, const Vector3D &v2 )
    {
      return Vector3D( v1.y() * v2.z() - v1.z() * v2.y(),
                          v1.z() * v2.x() - v1.x() * v2.z(),
                          v1.x() * v2.y() - v1.y() * v2.x() );
    }

    //! Returns the length of the vector
    double length() const
    {
      return std::sqrt( mX * mX + mY * mY + mZ * mZ );
    }

    //! Normalizes the current vector in place.
    void normalize()
    {
      const double len = length();
      if ( !qgsDoubleNear( len, 0.0 ) )
      {
        mX /= len;
        mY /= len;
        mZ /= len;
      }
    }

    Vector3D normalized() const
    {
      const float len = length();
      return qFuzzyIsNull(len - 1.0f) ? *this : qFuzzyIsNull(len) ? Vector3D()
                                                                  : Vector3D(mX / len, mY / len, mZ / len);
    }

    //! Returns the distance with the \a other Vector3D
    double distance( const Vector3D &other ) const
    {
      return std::sqrt( ( mX - other.x() ) * ( mX - other.x() ) +
                        ( mY - other.y() ) * ( mY - other.y() ) +
                        ( mZ - other.z() ) * ( mZ - other.z() ) );
    }

    //! Returns the perpendicular point of vector \a vp from [\a v1 - \a v2]
    static Vector3D perpendicularPoint( const Vector3D &v1, const Vector3D &v2, const Vector3D &vp )
    {
      const Vector3D d = ( v2 - v1 ) / v2.distance( v1 );
      const Vector3D v = vp - v2;
      const double t = dotProduct( v, d );
      Vector3D P = v2 + ( d * t );
      return P;
    }

#if 0
    /**
     * Returns a string representation of the 3D vector.
     * Members will be truncated to the specified \a precision.
     */
    QString toString( int precision = 17 ) const
    {
      QString str = "Vector3D (";
      str += qgsDoubleToString( mX, precision );
      str += ", ";
      str += qgsDoubleToString( mY, precision );
      str += ", ";
      str += qgsDoubleToString( mZ, precision );
      str += ')';
      return str;
    }
#endif

    /**
     * Converts the current object to QVector3D
     * \warning the conversion may decrease the accuracy (double to float values conversion)
     * \since QGIS 3.24
     */
    QVector3D toVector3D() const { return QVector3D( static_cast< float >( mX ), static_cast< float >( mY ), static_cast< float >( mZ ) ); }

    friend inline Vector3D operator-(Vector3D vector)
    {
      return Vector3D(-vector.mX, -vector.mY, -vector.mZ);
    }

  private:
    double mX = 0, mY = 0, mZ = 0;
};

#include <QDebug>

inline QDebug operator<<(QDebug dbg, Vector3D vector)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Vector3D("
                  << vector.x() << ", " << vector.y() << ", " << vector.z() << ')';
    return dbg;
}

#endif // VECTOR3D_H
