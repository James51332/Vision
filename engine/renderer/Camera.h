#pragma once

#include <glm/glm.hpp>

namespace Vision
{

/// Orthographic camera class to be used with particle shader.
class OrthoCamera
{
public:
  OrthoCamera(float windowWidth, float windowHeight, float size = 5.0f);
  // ~Camera();// No need to destroy any resources

  // Updates the camera based on user input
  void Update(float timestep);
  
  void SetPosition(const glm::vec3& position);
  void SetRotation(float rotation);
  void SetOrthographicSize(float size);
  void SetWindowSize(float windowWidth, float windowHeight);

  const glm::vec3& GetPosition() const { return m_Position; }
  float GetRotation() const { return m_Rotation; }
  float GetOrthographicSize() const { return m_OrthographicSize; }
  const glm::vec2& GetWindowSize() const { return m_WindowSize; }
  
  glm::vec2 GetMouseInWorldSpace() const;

  const glm::mat4& GetViewMatrix() const { return m_View; }
  const glm::mat4& GetProjectionMatrix() const { return m_Projection; }
  const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjection; }

private:
  // Return true when matrices need to be recalculated
  bool HandleMoving(float timestep);
  bool HandlePanning(float timestep);
  bool HandleZooming(float timestep);
  
private:
  void CalculateMatrices();
private:
  glm::vec3 m_Position = glm::vec3(0.0f);
  float m_Rotation = 0.0f;
  float m_OrthographicSize;
  glm::vec2 m_WindowSize;
  
  // We'll cache the aspect ratio too.
  float m_Aspect;

  glm::mat4 m_Projection;
  glm::mat4 m_View;
  glm::mat4 m_ViewProjection;
  
  // Cached to save cpu cycles
  glm::mat4 m_ViewProjectionInverse;
};

// Perspective Camera class with built in controller
class PerspectiveCamera
{
public:
  PerspectiveCamera(float windowWidth, float windowHeight, float near = 0.1f, float far = 100.0f);

  // Updates the camera based on user input
  void Update(float timestep);

  void SetPosition(const glm::vec3 &position);
  void SetPitch(float pitch);
  void SetYaw(float yaw);
  void SetFOV(float fov);
  void SetClip(float near, float far);
  void SetWindowSize(float windowWidth, float windowHeight);

  const glm::vec3 &GetPosition() const { return m_Position; }
  float GetPitch() const { return m_Pitch; }
  float GetYaw() const { return m_Yaw; }
  float GetFOV() const { return m_FOV; }
  float GetNear() const { return m_Near; }
  float GetFar() const { return m_Far; }
  const glm::vec2 &GetWindowSize() const { return m_WindowSize; }

  glm::vec3 GetMouseDirectionInWorldSpace() const;

  const glm::mat4 &GetViewMatrix() const { return m_View; }
  const glm::mat4 &GetProjectionMatrix() const { return m_Projection; }
  const glm::mat4 &GetViewProjectionMatrix() const { return m_ViewProjection; }

private:
  bool HandleMoving(float timestep);
  bool HandleTurning(float timestep);

private:
  void CalculateMatrices();

private:
  glm::vec3 m_Position = glm::vec3(0.0f);
  float m_Pitch = 0.0f, m_Yaw = 0.0f;
  float m_FOV = 45.0f;
  float m_Near = 0.1f, m_Far = 100.0f;
  glm::vec2 m_WindowSize;

  // We'll cache the aspect ratio too.
  float m_Aspect;

  glm::mat4 m_Projection;
  glm::mat4 m_View;
  glm::mat4 m_ViewProjection;

  glm::mat4 m_Transform;

  // Cached to save cpu cycles
  glm::mat4 m_ViewProjectionInverse;
};

}
