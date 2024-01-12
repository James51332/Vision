#type vertex
#version 410 core

layout (location = 0) in vec3 a_Pos;

uniform mat4 u_ViewProjection;
uniform vec3 u_CameraPos;
uniform float u_Time;

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec3 v_CamPos;

struct Wave
{
  vec2 origin;
  vec2 direction;
  vec4 scale; // amplitude, period, frequency, dummy
};

layout (std140) uniform WaveProperties
{ 
  Wave waves[100];
} waves;

void main()
{
  float offset = 0.0;
  vec3 gradientX = vec3(1.0, 0.0, 0.0);
  vec3 gradientZ = vec3(0.0, 0.0, 1.0);
    
  // Iterate over all waves
  for (int index = 0; index < 15; index++)
  {
    //if (index == 3) continue;

    Wave wave = waves.waves[index];

    // Get component of the position in the direction of the wave
    vec2 relPos = a_Pos.xz - wave.origin;
    float waveComponent = dot(relPos, wave.direction); 

    // Offset the plane by the wave
    float waveInput = (6.283 / wave.scale.y) * waveComponent - (wave.scale.z * u_Time) + wave.scale.w;
    offset += wave.scale.x * sin(waveInput);

    // Calculate slope of tangent and resolve into x and z component
    float tangentSlope = 6.283 * (wave.scale.x / wave.scale.y) * cos(waveInput); 
    gradientX.y += tangentSlope * wave.direction.x;
    gradientZ.y += tangentSlope  * wave.direction.y;
  }

  vec3 pos = a_Pos;
  pos.y += offset;

  v_Normal = cross(gradientZ, gradientX);
  v_WorldPos = a_Pos;
  v_CamPos = u_CameraPos;
  gl_Position = u_ViewProjection * vec4(pos, 1.0);
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
  vec3 diffuse =max(dot(norm, lightDir), 0) * 0.7 * lightColor;
  
  // Ambient
  vec3 ambient = 0.5 * lightColor;

  // Specular
  vec3 reflectDir = reflect(-lightDir, norm);  
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
  vec3 specular = 0.8 * spec * lightColor;  

  // Out Color
  vec3 color = (ambient + diffuse + specular) * vec3(0.2, 0.2, 0.6);
  f_FragColor = vec4(color, 1.0);
}