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

layout(vertices = 4) out;

out vec2 UV[];

void main()
{
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
  UV[gl_InvocationID] = v_UV[gl_InvocationID];

  if (gl_InvocationID == 0)
  {
    gl_TessLevelInner[0] = 16.0;
    gl_TessLevelInner[1] = 16.0;

    gl_TessLevelOuter[0] = 16.0;
    gl_TessLevelOuter[1] = 16.0;
    gl_TessLevelOuter[2] = 16.0;
    gl_TessLevelOuter[3] = 16.0;
  }
}

#type tes
#version 410 core

layout(quads, equal_spacing, ccw) in;

in vec2 UV[];

uniform mat4 u_ViewProjection;
uniform sampler2D heightMap;

out float height;

void main()
{
  // get the tess coords from tesselation generator
  float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;

  // quads are inputted ccw
  vec2 uv00 = UV[0]; // bottom left
  vec2 uv01 = UV[1]; // bottom right
  vec2 uv11 = UV[2]; // top right
  vec2 uv10 = UV[3]; // top left

  // calculate the uv coordinate of the tesselated point using bilinear interpolation
  vec2 t0 = (uv01 - uv00) * u + uv00;
  vec2 t1 = (uv11 - uv10) * u + uv10;
  vec2 texCoord = (t1 - t0) * v + t0;

  // sample the texture map
  height = texture(heightMap, texCoord).r;

  // interpolate to get world space position
  vec4 p00 = gl_in[0].gl_Position;
  vec4 p01 = gl_in[1].gl_Position;
  vec4 p11 = gl_in[2].gl_Position;
  vec4 p10 = gl_in[3].gl_Position;

  // bilinearly interpolate position coordinate across patch
  vec4 p0 = (p01 - p00) * u + p00;
  vec4 p1 = (p11 - p10) * u + p10;
  vec4 pos = (p1 - p0) * v + p0;

  // displace point along normal
  pos.y += height * 5.0;

  // transform to clip space
  gl_Position = u_ViewProjection * pos;
}

#type fragment
#version 410 core

in float height;

out vec4 FragColor;

void main()
{
  float color = height;
  FragColor = vec4(color, color, color, 1.0);
}