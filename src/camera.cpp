//
// Copyright (c) Mario Garcia, MIT License.
// 
#include "camera.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace pbr {


void Camera::Move(Camera::Movement movement, float dt)
{
  float dist = mSpeed * dt;
  switch (movement) {
    case Camera::FORWARD:
    {
      mPosition += mFront * dist; 
    }
    break;
    case Camera::BACK:
    {
      mPosition -=  mFront * dist;
    } 
    break; 
    case Camera::LEFT:
    {
      mPosition -= mRight * dist;
    } 
    break;
    case Camera::RIGHT:
    {
      mPosition += mRight * dist;
    } 
    break;
    case Camera::UP:
    {
      mPosition += mUp * dist;
    } 
    break;
    case Camera::DOWN:
    {
      mPosition -= mUp * dist;
    } 
    break;
    default: // do nothing
    break;
  }
}


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