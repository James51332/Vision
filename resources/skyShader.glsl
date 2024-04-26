#type vertex
#version 450 core

layout (location = 0) in vec3 a_Pos;

layout (push_constant) uniform pushConstants
{
  mat4 u_View;
  mat4 u_Projection;
  mat4 u_ViewProjection;
  vec2 u_ViewportSize;
  float u_Time;
};

out vec3 texCoord;

void main()
{
  mat4 noTranslateView = mat4(mat3(u_View));
  texCoord = a_Pos;
  vec4 pos = u_Projection * noTranslateView * vec4(a_Pos, 1.0);
  gl_Position = pos.xyww;
}

#type fragment
#version 450 core

in vec3 texCoord;

out vec4 FragColor;

layout (binding = 0) uniform samplerCube skybox;

void main()
{
  FragColor = texture(skybox, texCoord);
}