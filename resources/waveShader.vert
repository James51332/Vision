#version 450 core

layout(location = 0) in vec3 a_Pos;

layout(location = 0) uniform mat4 u_ViewProjection;
layout(location = 1) uniform vec3 u_CameraPos;
layout(location = 2) uniform float u_Time;
layout(location = 3) uniform int u_Waves;

layout(location = 0) out vec3 v_WorldPos;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec3 v_CamPos;

struct Wave {
  vec2 origin;
  vec2 direction;
  vec4 scale; // amplitude, period, frequency, phase
};

layout(std140, binding = 0) uniform WaveProperties {
  Wave waves[100];
} waves;

void main() {
  float offset = 0.0;
  vec3 gradientX = vec3(1.0, 0.0, 0.0);
  vec3 gradientZ = vec3(0.0, 0.0, 1.0);
  float ampSum = 0.0;
  vec3 pos = a_Pos;

  // Iterate over all waves
  for(int index = 0; index < u_Waves; index++) {
    Wave wave = waves.waves[index];

    // Get component of the position in the direction of the wave
    vec2 relPos = pos.xz - wave.origin;
    float wavePos = dot(relPos, wave.direction); 

    // Offset the plane by the wave
    float waveInput = (6.283 / wave.scale.y) * wavePos - (wave.scale.z * u_Time) + wave.scale.w;
    float waveHeight = wave.scale.x * exp(sin(waveInput) - 1);
    offset += waveHeight;
    ampSum += wave.scale.x;

    // Calculate slope of tangent and resolve into x and z component
    float tangentSlope = waveHeight * cos(waveInput) * (6.283 / wave.scale.y);
    gradientX.y += tangentSlope * wave.direction.x;
    gradientZ.y += tangentSlope * wave.direction.y;

    // Domain warping
    pos.x += tangentSlope * wave.direction.x / 100;
    pos.z += tangentSlope * wave.direction.y / 100;
  }

  offset /= ampSum;
  gradientX.y /= ampSum;
  gradientZ.y /= ampSum;
  pos.y += offset;

  v_Normal = cross(gradientZ, gradientX);
  v_WorldPos = pos;
  v_CamPos = u_CameraPos;
  gl_Position = u_ViewProjection * vec4(pos, 1.0);
}