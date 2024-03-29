#type vertex
#version 410 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec4 a_Color;
layout (location = 3) in vec2 a_UV;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform vec3 u_CameraPos;
uniform float u_Time;

out vec2 v_UV;
out vec3 v_Normal;

void main()
{
  gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
  
  v_UV = a_UV;
  v_Normal = a_Normal;
}

#type fragment
#version 410 core

in vec2 v_UV;
in vec3 v_Normal;

out vec4 FragColor;

void main()
{
  FragColor = vec4(vec3(1.0, 1.0, 1.0) * (0.4 + max(dot(v_Normal, vec3(0.800, 0.48, 0.36) * 0.3), 0)), 1.0);
}