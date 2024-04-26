#type vertex
#version 450 core

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

#type hull
#version 450 core

in vec2 v_UV[];

layout(vertices = 4) out;

out vec2 UV[];

layout (binding = 0) uniform sampler2D heightMap;

layout (push_constant) uniform pushConstants
{
  mat4 u_View;
  mat4 u_Projection;
  mat4 u_ViewProjection;
  vec2 u_ViewportSize;
  float u_Time;
};

float distanceTess(vec4 p0, vec4 p1, vec2 t0, vec2 t1)
{
  float g_TriangleTargetWidth = 20.0;

  // calculate the y coordinates to get dist
  p0.y += texture(heightMap, t0).r * 2.0;
  p1.y += texture(heightMap, t1).r * 2.0;

  // find center
  vec4 center = (p0 + p1) * 0.5;
  float radius = distance(p0, p1) / 2.0;

  // transform points to eye space
  vec4 sc0 = u_View * center;
	vec4 sc1 = sc0;
	sc0.x -= radius;
	sc1.x += radius;
  
	// project to clip space
	vec4 clip0 = u_Projection * sc0;
	vec4 clip1 = u_Projection * sc1;

  // normalize
	clip0 /= clip0.w;
	clip1 /= clip1.w;

  // convert to pixel space
	clip0.xy *= u_ViewportSize;
	clip1.xy *= u_ViewportSize;

  // find distance
	float d = distance(clip0, clip1);

	// g_tessellatedTriWidth is desired pixels per tri edge
	return clamp(d / g_TriangleTargetWidth, 0,64);
}

void main()
{
  // pass through texture coordinates and world coordinates
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
  UV[gl_InvocationID] = v_UV[gl_InvocationID];

  // only run the tesselation computation for one vertex in the patch
  if (gl_InvocationID == 0)
  {
    gl_TessLevelOuter[1] = distanceTess(gl_in[0].gl_Position, gl_in[1].gl_Position, v_UV[0], v_UV[1]);
    gl_TessLevelOuter[2] = distanceTess(gl_in[1].gl_Position, gl_in[2].gl_Position, v_UV[1], v_UV[2]);
    gl_TessLevelOuter[3] = distanceTess(gl_in[2].gl_Position, gl_in[3].gl_Position, v_UV[2], v_UV[3]);
    gl_TessLevelOuter[0] = distanceTess(gl_in[3].gl_Position, gl_in[0].gl_Position, v_UV[3], v_UV[0]);

    // take average of opposite edges to get inside
    gl_TessLevelInner[0] = (gl_TessLevelOuter[1] + gl_TessLevelOuter[3]) * 0.5;
    gl_TessLevelInner[1] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[2]) * 0.5;
  }
}

#type domain
#version 450 core

layout(quads, fractional_odd_spacing, ccw) in;

in vec2 UV[];

layout (push_constant) uniform pushConstants
{
  mat4 u_View;
  mat4 u_Projection;
  mat4 u_ViewProjection;
  vec2 u_ViewportSize;
  float u_Time;
};

layout (binding = 0) uniform sampler2D heightMap;

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
  pos.y += height * 2.0;

  // transform to clip space
  gl_Position = u_ViewProjection * pos;
}

#type fragment
#version 450 core

in float height;

out vec4 FragColor;

void main()
{
  float color = height;
  FragColor = vec4(color, color, color, 1.0);
}