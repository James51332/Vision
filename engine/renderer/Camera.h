#pragma once

#include <glm/glm.hpp>

namespace Vision
{

class Camera
{
public:
  Camera(float windowWidth, float windowHeight)
    : m_WindowSize({windowWidth, windowHeight}), m_Aspect(windowHeight / windowWidth) {}
  virtual ~Camera() = default;

  // Update Movement
  virtual void Update(float timestep) = 0;

  // Matrix Access Methods
  const glm::mat4 &GetViewMatrix() const { return m_View; }
  const glm::mat4 &GetProjectionMatrix() const { return m_Projection; }
  const glm::mat4 &GetViewProjectionMatrix() const { return m_ViewProjection; }

  void SetPosition(const glm::vec3& position);
  void SetRotation(const glm::vec3& rotation);
  void SetWindowSize(float width, float height);

  const glm::vec3& GetPosition() const { return m_Position; }
  const glm::vec3& GetRotation() const { return m_Rotation; }
  const glm::vec2 &GetWindowSize() const { return m_WindowSize; }

protected:
  virtual void CalculateMatrices() = 0;

protected:
  glm::vec3 m_Position;
  glm::vec3 m_Rotation; // pitch, yaw, and roll
  
  glm::vec2 m_WindowSize;
  float m_Aspect;

  // Matrices
  glm::mat4 m_Transform;
  glm::mat4 m_View;
  glm::mat4 m_Projection;
  glm::mat4 m_ViewProjection;
  glm::mat4 m_ViewProjectionInverse;
};

/// Orthographic camera class to be used with particle shader.
class OrthoCamera : public Camera
{
public:
  OrthoCamera(float windowWidth, float windowHeight, float size = 5.0f);

  // Updates the camera based on user input
  void Update(float timestep);

  void SetOrthographicSize(float size);
  float GetOrthographicSize() const { return m_OrthographicSize; }  

  glm::vec2 GetMouseInWorldSpace() const;

private:
  // Return true when matrices need to be recalculated
  bool HandleMoving(float timestep);
  bool HandlePanning(float timestep);
  bool HandleZooming(float timestep);
  
protected:
  void CalculateMatrices();

private:
  float m_OrthographicSize;
};

// Perspective Camera class with built in controller
class PerspectiveCamera : public Camera
{
public:
  PerspectiveCamera(float windowWidth, float windowHeight, float near = 0.1f, float far = 100.0f);

  // Updates the camera based on user input
  void Update(float timestep);

  void SetFOV(float fov);
  void SetClip(float near, float far);
  void SetMoveSpeed(float speed);

  float GetFOV() const { return m_FOV; }
  float GetNear() const { return m_Near; }
  float GetFar() const { return m_Far; }
  float GetMoveSpeed() const { return m_MoveSpeed; }

  glm::vec3 GetMouseDirectionInWorldSpace() const;

private:
  bool HandleMoving(float timestep);
  bool HandleTurning(float timestep);

protected:
  void CalculateMatrices();

private:
  float m_FOV = 45.0f;
  float m_Near = 0.1f, m_Far = 100.0f;
  float m_MoveSpeed = 3.0f;
};

}
