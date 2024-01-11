#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <SDL.h>

#include "core/Input.h"

namespace Vision
{

OrthoCamera::OrthoCamera(float windowWidth, float windowHeight, float size)
  : m_WindowSize(windowWidth, windowHeight), m_Aspect(windowWidth / windowHeight), m_OrthographicSize(size)
{
  CalculateMatrices();
}

void OrthoCamera::Update(float timestep)
{
  // Store whether we've moved so we don't wastefully calculate matrices
  bool shouldUpdateMatrices = false;
  
  shouldUpdateMatrices |= HandleMoving(timestep);
  shouldUpdateMatrices |= HandlePanning(timestep);
  shouldUpdateMatrices |= HandleZooming(timestep);
  
  // Recalculate matrices as needed
  if (shouldUpdateMatrices)
    CalculateMatrices();
}

void OrthoCamera::SetPosition(const glm::vec3& position)
{
  m_Position = position;
  CalculateMatrices();
}

void OrthoCamera::SetRotation(float rotation)
{
  m_Rotation = rotation;
  CalculateMatrices();
}

void OrthoCamera::SetOrthographicSize(float size)
{
  m_OrthographicSize = size;
  CalculateMatrices();
}

void OrthoCamera::SetWindowSize(float windowWidth, float windowHeight)
{
  m_WindowSize = { windowWidth, windowHeight };
  m_Aspect = windowWidth / windowHeight;
  CalculateMatrices();
}

glm::vec2 OrthoCamera::GetMouseInWorldSpace() const
{
  // To convert from screen space to world space we'll need to undo all
  // of the transformations that happen in the rendering pipeline.
  // Step 1) We convert from pixels to OpenGL's coordinates (-1 to 1 on x and y axes)
  glm::vec2 mouseCoordinates = { Input::GetMouseX(), m_WindowSize.y - Input::GetMouseY() }; // (0, width); (0, height)
  mouseCoordinates = { (mouseCoordinates.x  / m_WindowSize.x), (mouseCoordinates.y / m_WindowSize.y) }; // (0, 1); (0, 1)
  mouseCoordinates = 2.0f * mouseCoordinates - 1.0f; // (-1, 1); (-1, 1)
  
  // Step 2) We multiply by the inverse of the view projection matrix (need a glm::vec4 to do this)
  glm::vec4 mouseCoord4 = { mouseCoordinates.x, mouseCoordinates.y, 0.0f, 1.0f };
  mouseCoord4 = m_ViewProjectionInverse * mouseCoord4;
  
  return { mouseCoord4.x, mouseCoord4.y };
}

bool OrthoCamera::HandleMoving(float timestep)
{
  bool shouldUpdateMatrices = false;
  
  // Handling sliding with WASD (TODO: Camera right vs World right?)
  float moveSpeed = m_OrthographicSize * 1.5f;
  if (Input::KeyDown(SDL_SCANCODE_W))
  {
    m_Position.y += moveSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_A))
  {
    m_Position.x -= moveSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_S))
  {
    m_Position.y -= moveSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_D))
  {
    m_Position.x += moveSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  
  // Handle rotation
  float rotationSpeed = 200.0f;
  if (Input::KeyDown(SDL_SCANCODE_LEFT))
  {
    m_Rotation += rotationSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_RIGHT))
  {
    m_Rotation -= rotationSpeed * timestep;
    shouldUpdateMatrices = true;
  }
  
  return shouldUpdateMatrices;
}

bool OrthoCamera::HandlePanning(float timestep)
{
  static glm::vec2 lastMousePos;
  if (Input::MousePress(SDL_BUTTON_LEFT)) // Begin panning
  {
    lastMousePos = GetMouseInWorldSpace();
  }
  else if (Input::MouseDown(SDL_BUTTON_LEFT)) // Update panning
  {
    glm::vec2 curMousePos = GetMouseInWorldSpace();
    glm::vec2 delta = curMousePos - lastMousePos;
    lastMousePos = curMousePos - delta; // update our old mouse position to new coordinates
    
    m_Position.x -= delta.x;
    m_Position.y -= delta.y;
    return true;
  }
  
  return false;
}

bool OrthoCamera::HandleZooming(float timestep)
{
  // Scrolling zoom for now (TODO: pinch zoom in the future?)
  float scroll = Input::GetScrollY();
  if (scroll != 0)
  {
    // TODO: Zoom speed based on scroll speed? (using exponentiation)
    float zoomSize = 1.0f;
    if (scroll > 0)
      zoomSize = 0.9f;
    else if (scroll < 0)
      zoomSize = 1.1f;
    
    // Scale the camera
    m_OrthographicSize *= zoomSize;
    
    glm::vec2 zoomPos = GetMouseInWorldSpace(); // lock in around this point
    glm::vec2 camPos = { m_Position.x, m_Position.y };
    
    // This distance must stay the same in screen space to keep the mouse focused
    // around the same point in world space. Thus, it must be scaled proportionally
    // to orthographic size, and we do this by moving the camera the same proportion
    // of this distance as the orthographic size is scaled. If we make this line 3x
    // bigger by zooming in to 0.333x, then we must move along 66.7% of this line so that
    // the distance stays the same.
    glm::vec2 delta = zoomPos - camPos;
    glm::vec2 camDelta = (1.0f - zoomSize) * delta;
    
    m_Position.x += camDelta.x;
    m_Position.y += camDelta.y;
    return true;
  }
  
  return false;
}

void OrthoCamera::CalculateMatrices()
{
  // Calculate the view matrix (translates from world space to camera space-inverse of camera's transform)
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
                      * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f)); // rotate and then translate
  
  m_View = glm::inverse(transform);

  // Calculate the projection matrix (use glm::ortho)
  float left, right, top, bottom;
  top = m_OrthographicSize;
  bottom = -m_OrthographicSize;
  left = -m_OrthographicSize * m_Aspect;
  right = m_OrthographicSize * m_Aspect;

  m_Projection = glm::ortho(left, right, bottom, top);

  // Cache the view projection matrix in a variable as well (first view then projection-translate then stretch)
  m_ViewProjection = m_Projection * m_View;
  
  // We cache this for converting from normalized space to world space.
  m_ViewProjectionInverse = glm::inverse(m_ViewProjection);
}

// Perspective Camera
PerspectiveCamera::PerspectiveCamera(float windowWidth, float windowHeight, float near, float far)
  : m_WindowSize({windowWidth, windowHeight}), m_Near(near), m_Far(far)
{
  CalculateMatrices();
}

void PerspectiveCamera::Update(float timestep)
{
  // Store whether we've moved so we don't wastefully calculate matrices
  bool shouldUpdateMatrices = false;

  shouldUpdateMatrices |= HandleMoving(timestep);
  shouldUpdateMatrices |= HandleTurning(timestep);

  // Recalculate matrices as needed
  if (shouldUpdateMatrices)
    CalculateMatrices();
}

bool PerspectiveCamera::HandleMoving(float timestep)
{
  bool update = false;

  glm::vec3 forwardDirection = -m_Transform[2];
  glm::vec3 rightDirection = m_Transform[0];

  float moveSpeed = 3.0f;
  if (Input::KeyDown(SDL_SCANCODE_GRAVE)) moveSpeed = 10.0f;

  // Moving Up and Side to Side
  if (Input::KeyDown(SDL_SCANCODE_W)) 
  {
    m_Position += forwardDirection * moveSpeed * timestep;
    update = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_S))
  {
    m_Position -= forwardDirection * moveSpeed * timestep;
    update = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_A))
  {
    m_Position -= rightDirection * moveSpeed * timestep;
    update = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_D))
  {
    m_Position += rightDirection * moveSpeed * timestep;
    update = true;
  }

  // Vertical 
  if (Input::KeyDown(SDL_SCANCODE_SPACE))
  {
    m_Position += glm::vec3(0.0f, 1.0f, 0.0f) * moveSpeed * timestep;
    update = true;
  }
  if (Input::KeyDown(SDL_SCANCODE_LSHIFT))
  {
    m_Position -= glm::vec3(0.0f, 1.0f, 0.0f) * moveSpeed * timestep;
    update = true;
  }

  return update;
}

bool PerspectiveCamera::HandleTurning(float timestep)
{
  bool update = false;

  static float mouseX = 0, mouseY = 0;
  float dx = Input::GetMouseX() - mouseX;
  float dy = Input::GetMouseY() - mouseY;
  mouseX = Input::GetMouseX();
  mouseY = Input::GetMouseY();

  if (Input::MouseDown(SDL_BUTTON_LEFT))
  {
    update = true;

    m_Yaw -= dx / 3.0f;
    m_Pitch -= dy / 3.0f;
  }

  return update;
}

void PerspectiveCamera::SetPosition(const glm::vec3 &position)
{
  m_Position = position;
  CalculateMatrices();
}

void PerspectiveCamera::SetPitch(float pitch)
{
  m_Pitch = pitch;
  CalculateMatrices();
}

void PerspectiveCamera::SetYaw(float yaw)
{
  m_Yaw = yaw;
  CalculateMatrices();
}

void PerspectiveCamera::SetFOV(float fov)
{
  m_FOV = fov;
  CalculateMatrices();
}

void PerspectiveCamera::SetClip(float near, float far)
{
  m_Near = near;
  m_Far = far;
  CalculateMatrices();
}

void PerspectiveCamera::SetWindowSize(float windowWidth, float windowHeight)
{
  m_WindowSize = {windowWidth, windowHeight};
  CalculateMatrices();
}

glm::vec3 PerspectiveCamera::GetMouseDirectionInWorldSpace() const
{
  // TODO

  return { 0.0f, 0.0f, 1.0f };
}

void PerspectiveCamera::CalculateMatrices()
{
  // Calculate the view matrix (translates from world space to camera space-inverse of camera's transform)
  m_Transform = glm::translate(glm::mat4(1.0f), m_Position)
                      * glm::rotate(glm::mat4(1.0f), glm::radians(m_Yaw), glm::vec3(0.0f, 1.0f, 0.0f)) // yaw
                      * glm::rotate(glm::mat4(1.0f), glm::radians(m_Pitch), glm::vec3(1.0f, 0.0f, 0.0f)); // pitch

  m_View = glm::inverse(m_Transform);

  // Calculate the projection matrix (use glm::ortho)
  m_Aspect = m_WindowSize.x / m_WindowSize.y;
  m_Projection = glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);

  // Cache the view projection matrix in a variable as well (first view then projection-translate then stretch)
  m_ViewProjection = m_Projection * m_View;

  // We cache this for converting from normalized space to world space.
  m_ViewProjectionInverse = glm::inverse(m_ViewProjection);
}

}
