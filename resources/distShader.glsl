#type vertex
#version 410 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec4 a_Color;
layout (location = 3) in vec2 a_UV;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform sampler2D heightMap; 

out vec2 uv;

void main()
{
  vec3 position = vec3(u_Transform * vec4(a_Position, 1.0));
  position.y += texture(heightMap, a_UV).r * 0.5;
  gl_Position = u_ViewProjection * vec4(position, 1.0);

  uv = a_UV;
}

#type fragment
#version 410 core

in vec2 uv;

out vec4 FragColor;

uniform sampler2D heightMap; 

void main()
{
  float color = (texture(heightMap, uv).r + 1.0) / 2.0;
  FragColor = vec4(color, color, color, 1.0);
}