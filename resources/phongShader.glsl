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

out vec3 v_WorldPos;
out vec2 v_UV;
out vec3 v_Normal;

void main()
{
  v_WorldPos = vec3(u_Transform * vec4(a_Position.xzy, 1.0));
  gl_Position = u_ViewProjection * vec4(v_WorldPos, 1.0);
  
  v_UV = a_UV;
  v_Normal = a_Normal;
}

#type fragment
#version 410 core

in vec3 v_WorldPos;
in vec2 v_UV;
in vec3 v_Normal;

out vec4 FragColor;

uniform vec3 u_CameraPos;

void main()
{
  vec3 lightDir = normalize(vec3(1.0, -0.2, 1.0));

  float ambient = 0.2;
  float diffuse = max(dot(v_Normal, -lightDir), 0) * 0.4;

  vec3 camDir = normalize(u_CameraPos - v_WorldPos);
  float specular = pow(max(dot(reflect(camDir, v_Normal), -lightDir), 0), 4) * 0.4;

  vec3 color = vec3(1.0);
  float light = specular + diffuse + ambient;
  FragColor = vec4(color * light, 1.0);
}