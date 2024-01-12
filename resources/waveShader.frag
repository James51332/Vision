#version 450 core

layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec3 v_CamPos;

layout(location = 0) out vec4 f_FragColor;

void main() {
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
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = 0.5 * spec * lightColor;  

  // Out Color
  vec3 oceanColor = vec3(0.25, 0.61, 0.86);
  vec3 color = (ambient + diffuse + specular) * oceanColor;
  f_FragColor = vec4(color.xyz, 1.0);
}