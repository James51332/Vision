#type vertex
#version 450 core

layout (location = 0) in vec3 a_Pos;
layout (location = 3) in vec2 a_UV;

// We use uniform binding zero for engine resources. 
layout (binding = 0) uniform pushConstants
{
  mat4 u_View;
  mat4 u_Projection;
  mat4 u_ViewProjection;
  vec2 u_ViewportSize;
  float u_Time;
  float dummy; // 16 byte alignment
};

out vec2 texCoord;

void main()
{
  texCoord = a_UV;
  gl_Position = u_ViewProjection * vec4(a_Pos, 1.0);
}

#type fragment
#version 450 core

in vec2 texCoord;

out vec4 FragColor;

layout (binding = 0) uniform sampler2D iceland;

void main()
{
  FragColor = texture(iceland, texCoord.st);
}
