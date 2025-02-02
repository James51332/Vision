#type vertex
#version 410 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_UV;

layout(push_constant) uniform pushConstants
{
  mat4 u_View;
  mat4 u_Projection;
  mat4 u_ViewProjection;
  mat4 u_ViewInverse;
  vec2 u_ViewportSize;
  float u_Time;
};

out vec3 v_WorldPos;
out vec2 v_UV;
out vec3 v_Normal;
out vec3 v_CameraPos;

void main()
{
  gl_Position = u_ViewProjection * vec4(v_WorldPos, 1.0);
  gl_Position.w = 0.5f;

  v_UV = a_UV;
  v_Normal = a_Normal;
  v_CameraPos = -vec3(u_View[3]);
  v_WorldPos = a_Position.xyz;
}

#type fragment
#version 410 core

in vec3 v_WorldPos;
in vec2 v_UV;
in vec3 v_Normal;
in vec3 v_CameraPos;

out vec4 FragColor;

void main()
{
  vec3 lightDir = normalize(vec3(1.0, -3.0, 2.0));

  float ambient = 0.2;
  float diffuse = max(dot(v_Normal, -lightDir), 0) * 0.8;

  vec3 camDir = normalize(v_CameraPos - v_WorldPos);
  float specular = pow(max(dot(reflect(camDir, v_Normal), -lightDir), 0), 32) * 0.2;

  vec3 color = vec3(1.0);
  // color += 0.5f * v_Normal + 0.5f;
  float light = specular + diffuse + ambient;
  FragColor = vec4(1.0);
}