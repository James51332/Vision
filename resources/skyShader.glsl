#type vertex
#version 410 core

layout (location = 0) in vec3 a_Pos;

out vec3 texCoord;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
  mat3 view = mat3(u_View);
  texCoord = a_Pos;
  vec4 pos = u_Projection * mat4(view) * vec4(a_Pos, 1.0);
  gl_Position = pos.xyww;
}

#type fragment
#version 410 core

in vec3 texCoord;

out vec4 FragColor;

uniform samplerCube skybox;

void main()
{
  FragColor = texture(skybox, texCoord);
}