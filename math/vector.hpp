//
// Copyright (c) Mario Garcia, MIT License.
//
#ifndef __VECTOR_HPP
#define __VECTOR_HPP

#include <cmath>


namespace pbr {


template<typename _Type> class Vector3;
template<typename _Type> class Vector2;


template<typename _Value = float>
class Vector4 {
public:
  // Vector4 contruction.
  Vector4(_Value x = 0.0f,
          _Value y = 0.0f,
          _Value z = 0.0f,
          _Value w = 1.0f)
    : x(x), y(y), z(z), w(w) { }

  Vector4(Vector3 &v)
    : x(v.x), y(v.y), z(v.z), w(1.0f) { }

  Vector4(Vector2 &v)
    : x(v.x), y(v.y), z(0.0f), w(0.0f) { }

  Vector4 operator+(Vector4 &vec) {
    return Vector4(
      x + vec.x,
      y + vec.y,
      z + vec.z,
      w + vec.w
    );
  }

  Vector4 operator-(Vector4 &vec) {
    return Vector4(
      x - vec.x,
      y - vec.y,
      z - vec.z,
      w - vec.w
    );
  }

  Vector4 operator*(Vector4 &vec) {
    return Vector4(
      x * vec.x,
      y * vec.y,
      z * vec.z,
      w * vec.w
    );
  }

  Vector4 operator/(Vector4 &vec) {
    static_assert(
      (vec.x != 0 || vec.y != 0 || vec.z != 0 || vec.w != 0),
      "Divide by zero!"
      );

    return Vector4(
      x / vec.x,
      y / vec.y,
      z / vec.z,
      w / vec.w
    );
  }

  bool operator!=(Vector4 &vec) {
    return (x != vec.x) && (y != vec.y) && (z != vec.z) && (w != vec.w);
  }

  Vector4 Copy() {
    return Vector4(x, y, z, w);
  }


  union {
    struct { _Value x, y, z, w; };
    struct { _Value r, g, b, a; };
    struct { _Value s, t, r, q; };
  };
};


template<typename _Value = float>
class Vector3 {
public:
  Vector3(_Value x = 1.0f,
          _Value y = 1.0f,
          _Value z = 1.0f)
    : x(x), y(y), z(z) { }


  union {
    struct { _Value x, y, z; };
    struct { _Value r, g, b; };
    struct { _Value s, t, r; };
  };
};


template<typename _Value = float>
class Vector2 {
public:
  Vector2(_Value x = 1.0f,
          _Value y = 1.0f)
    : x(x), y(y) { }


  union {
    struct { _Value x, y; };
    struct { _Value r, g; };
    struct { _Value s, t; };
  };
};

template<typename _Type = float>
Vector3<_Type> Cross(Vector3<_Type> const &a, Vector3<_Type> const &b);

template<typename _Type = float>
Vector3<_Type> Dot(Vector3<_Type> const &a, Vector3<_Type> const &b);

template<typename _Type = float>
_Type Length(Vector3<_Type> const &a, Vector3<_Type> const &b) {
}


template<typename _Type = float>
_Type Magnitude(Vector3<_Type> const &a) {
  return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

template<typename _Type = float>
Vector3<_Type> Normalize(Vector3<_Type> &vec) {
  _Type mag = Magnitude(vec);
  return Vector3<_Type>(vec.x / mag, vec.y / mag, vec.z / mag);
}


typedef Vector3<float> Vec3;
typedef Vector2<float> Vec2;

#include <vector.inl>
} // pbr
#endif // __VECTOR_HPP