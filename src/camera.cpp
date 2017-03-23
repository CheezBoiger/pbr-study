//
// Copyright (c) Mario Garcia, MIT License.
// 
#include "camera.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace pbr {


void Camera::Update(double dt)
{
  mFront = glm::normalize(mLookat - mPosition);
  mProjection = glm::perspective(mFov, mAspect, mNear, mFar);
  glm::vec3 axis = glm::cross(mFront, mUp);
  glm::quat pitchQuat = glm::angleAxis(mPitch, axis);
  glm::quat yawQuat =  glm::angleAxis(mYaw, mUp);
  glm::quat q = glm::normalize(glm::cross(pitchQuat, yawQuat));
  mFront = glm::rotate(q, mFront);
  mRight = glm::normalize(glm::cross(mFront, glm::vec3(0.0f, 1.0f, 0.0f)));
  mUp = glm::normalize(glm::cross(mRight, mFront));
  mView =   glm::lookAt(mPosition, mFront + mPosition, mUp);
}
} // pbr