#type vertex
#version 410 core

layout (location = 0) in vec3 a_Pos;

uniform mat4 u_ViewProjection;
uniform vec3 u_CameraPos;
uniform float u_Time;
uniform mat4 u_Transform;
uniform int u_Waves;

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec3 v_CamPos;

struct Wave
{
  vec2 origin;
  vec2 direction;
  vec4 scale; // amplitude, period, frequency, phase
};

layout (std140) uniform WaveProperties
{ 
  Wave waves[100];
} waves;

void main()
{
  float offset = 0.0;
  vec3 gradientX = vec3(1.0, 0.0, 0.0);
  vec3 gradientY = vec3(0.0, 1.0, 0.0);
  float ampSum = 0.0;
  vec3 pos = a_Pos;
    
  // Iterate over all waves
  for (int index = 0; index < u_Waves; index++)
  {
    Wave wave = waves.waves[index];

    // Get component of the position in the direction of the wave
    float wavePos = dot(pos.xy - wave.origin, wave.direction); 

    // Offset the plane by the wave
    float waveInput = (6.283 / wave.scale.y) * wavePos - (wave.scale.z * u_Time) + wave.scale.w;
    float waveHeight = wave.scale.x * exp(sin(waveInput) - 1);
    offset += waveHeight;
    ampSum += wave.scale.x;

    // Calculate slope of tangent and resolve into x and z component
    float tangentSlope = waveHeight * cos(waveInput) * (6.283 / wave.scale.y); 
    gradientX.z += tangentSlope * wave.direction.x;
    gradientY.z += tangentSlope * wave.direction.y;

    // Domain warping
    pos.x += tangentSlope * wave.direction.x / 100;
    pos.y += tangentSlope * wave.direction.y / 100;
  }

  offset /= ampSum;
  gradientX.z /= ampSum;
  gradientY.z /= ampSum;
  pos.z += offset;

  v_Normal = cross(gradientY, gradientX);
  v_WorldPos = (u_Transform * vec4(pos, 1.0)).xyz;
  v_CamPos = u_CameraPos;
  gl_Position = u_ViewProjection * u_Transform * vec4(pos, 1.0);
}

#type fragment
#version 410 core

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec3 v_CamPos;

layout (location = 0) out vec4 f_FragColor;

void main()
{
  // Scene Info 
  vec3 lightPos = vec3(-2000.0, 2000.0, -2000.0);
  vec3 lightDir = normalize(lightPos - v_WorldPos);  
  vec3 lightColor = vec3(1.0);
  vec3 viewDir = normalize(v_CamPos - v_WorldPos);
  vec3 norm = normalize(v_Normal);

  // Diffuse
  vec3 diffuse = max(dot(norm, lightDir), 0) * 1.0 * lightColor;
  
  // Ambient
  vec3 ambient = 0.4 * lightColor;

  // Specular
  vec3 reflectDir = reflect(-lightDir, norm);  
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
  vec3 specular = 0.5 * spec * lightColor;  

  // Out Color
  vec3 oceanColor = vec3(0.25, 0.61, 0.86);
  vec3 color = (ambient + diffuse + specular) * oceanColor;
  f_FragColor = vec4(color.xyz, 1.0);
}