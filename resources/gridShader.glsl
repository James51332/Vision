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

out vec3 v_NearPos;
out vec3 v_FarPos;
out mat4 v_ViewProjection;

vec3 unprojectPoint(vec2 xy, float z, mat4 viewProj)
{
  vec4 worldPos = inverse(viewProj) * vec4(xy, z, 1); // unproject the point
  return worldPos.xyz / worldPos.w; // return the normalized vector
}

void main()
{
  // Step 1: Clip Space Quad (Provided By Mesh)
  gl_Position = vec4(a_Position, 1.0);

  // Step 2: Unprojection
  v_NearPos = unprojectPoint(a_Position.xy, -1.0, u_ViewProjection);
  v_FarPos = unprojectPoint(a_Position.xy, 1.0, u_ViewProjection);

  // Step 6: Depth (converts the world space point back to clip space)
  v_ViewProjection = u_ViewProjection;
}

#type fragment
#version 410 core

in vec3 v_NearPos;
in vec3 v_FarPos;
in mat4 v_ViewProjection;

out vec4 FragColor;

vec4 grid(vec3 worldPos)
{
  vec2 coord = worldPos.xz;
  vec2 derivative = fwidth(coord); // lines are thicker for smaller steps.
  vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
  float line = min(grid.x, grid.y);
  vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
  return color;
}

void main()
{
  // Step 3: Math
  float t = -v_NearPos.y / (v_FarPos.y - v_NearPos.y); // solve using parametric eq. w/ y = 0
  vec3 worldPos = v_NearPos + t * (v_FarPos - v_NearPos); // cast ray to find world space plane

  // Step 4: Grid
  FragColor = grid(worldPos) * float(t > 0); // only render if in view

  // Step 5: Fade
  float rayLength = length(t * (v_FarPos.xz - v_NearPos.xz));
  FragColor.a *= clamp(1.0 - rayLength / 50.0, 0.0, 1.0);

  // Step 6: Depth
  vec4 newClipPos = v_ViewProjection * vec4(worldPos, 1.0);
  gl_FragDepth = 0.5 + 0.5 * (newClipPos.z / newClipPos.w); // 0 to 1
}