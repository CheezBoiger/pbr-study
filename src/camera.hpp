//
// Copyright (c) Mario Garcia, MIT License.
//
#ifndef __CAMERA_HPP
#define __CAMERA_HPP

#include "platform.hpp"
#include "matrix.hpp"
#include "vector.hpp"


namespace pbr {


class Camera {
public:

  void Update(double dt);
  double GetSpeed() { return speed; }

  Mat4 GetProjection() { return projection; }
  Mat4 GetView() { return view; }

private:
  /// Z axis direction vector.
  Vec3 front;

  /// X axis direction vector. 
  Vec3 right;
  
  /// z axis direction vector.
  Vec3 up;

  /// The View of the camera.
  Mat4 view;

  /// The projection of the camera.
  Mat4 projection;

  /// speed of the camera.
  double speed;

  /// current pitch in degrees.
  double pitch;
};
} // pbr
#endif // __CAMERA_HPP