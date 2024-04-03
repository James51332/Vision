#type vertex
#version 410 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec4 a_Color;
layout (location = 3) in vec2 a_UV;

out vec2 v_UV;

void main()
{
  gl_Position = vec4(a_Position, 1.0);
  v_UV = a_UV;
}

#type tcs
#version 410 core

in vec2 v_UV[];

layout(vertices = 3) out;

out vec2 UV[];

void main()
{
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
  UV[gl_InvocationID] = v_UV[gl_InvocationID];

  if (gl_InvocationID == 0)
  {
    gl_TessLevelInner[0] = 16.0;

    gl_TessLevelOuter[0] = 16.0;
    gl_TessLevelOuter[1] = 16.0;
    gl_TessLevelOuter[2] = 16.0;
  }
}

#type tes
#version 410 core

layout(triangles, equal_spacing, ccw) in;

in vec2 UV[];

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform sampler2D heightMap;

out float height;

void main()
{
  // interpolate using barycentric coordinate system
  vec2 texCoord = vec2(0.0);
  texCoord += gl_TessCoord[0] * UV[0];
  texCoord += gl_TessCoord[1] * UV[1];
  texCoord += gl_TessCoord[2] * UV[2];

  // calculate the world space coordinate of the tesselated point
  vec4 pos = vec4(0.0);
  pos += gl_TessCoord[0] * gl_in[0].gl_Position;
  pos += gl_TessCoord[1] * gl_in[1].gl_Position;
  pos += gl_TessCoord[2] * gl_in[2].gl_Position;

  // rotate the plane
  pos.w = 1.0; // since we added all components before
  pos = u_Transform * pos;

  // sample the texture map
  height = texture(heightMap, texCoord).r;

  // calculate the world space pos
  pos.y += height;

  // transform to clip space
  gl_Position = u_ViewProjection * pos;
}

#type fragment
#version 410 core

in float height;

out vec4 FragColor;

void main()
{
  float color = (height + 1.0) / 2.0;
  FragColor = vec4(color, color, color, 1.0);
}