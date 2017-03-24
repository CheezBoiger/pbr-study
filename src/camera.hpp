//
// Copyright (c) Mario Garcia, MIT License.
//
#ifndef __CAMERA_HPP
#define __CAMERA_HPP

#include "platform.hpp"
#include "vertex.hpp"


namespace pbr {


class Camera {
public:
  enum Movement;
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f))
    : mPosition(position)
  { }

  void Update(double dt);
  double GetSpeed() { return mSpeed; }
  void Move(Movement movement, double dt);
  void SetLookAt(glm::vec3 target) { mLookat = target; }
  void SetPosition(glm::vec3 pos) { mPosition = pos; }
  void SetFov(float fov) { mFov = fov; }
  void SetAspect(float aspect) { mAspect = aspect; }
  void SetNearFar(float n, float f) { mNear = n; mFar = f; }  
  void SetSpeed(float speed) { mSpeed = speed; }

  glm::mat4 GetProjection() { return mProjection; }
  glm::mat4 GetView() { return mView; }
  glm::vec3 GetPosition() { return mPosition; }

private:
  /// Z axis direction vector.
  /// Set to be local to the camera position.
  glm::vec3 mFront = glm::vec3(0.0f, 0.0f, -1.0f);

  /// X axis direction vector. 
  /// Set to be local to the camera position.
  glm::vec3 mRight = glm::vec3(1.0f, 0.0f, 0.0f);
  
  /// z axis direction vector.
  /// Set to be local to the camera position.
  glm::vec3 mUp = glm::vec3(0.0f, 1.0f, 0.0f);

  /// The View of the camera.
  glm::mat4 mView;

  /// The projection of the camera.
  glm::mat4 mProjection;

  glm::vec3 mPosition;

  /// speed of the camera.
  float mSpeed;

  /// current pitch in degrees.
  float mPitch            = 0.0f;
  float mYaw              = 0.0f;
  float mFov;
  float mAspect;
  float mNear;
  float mFar;

  glm::vec3 mLookat;

public:
  enum Movement {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FORWARD,
    BACK
  };
};
} // pbr
#endif // __CAMERA_HPP