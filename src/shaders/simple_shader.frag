#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPositionWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 0) out vec4 outColor;
struct PointLight{
  vec4 position;
  vec4 color;
};
layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  vec4 ambientLightColor;
  PointLight pointlights[10];
  int numLights;

} ubo;


layout(push_constant) uniform Push {
  mat4 modelMatrix; // projection * view * model
  mat4 normalMatrix;
} push;

void main() {

  vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 surfaceNormal = normalize(fragNormalWorld);
  for(int i = 0; i < ubo.numLights; i++){
    PointLight pointLight = ubo.pointlights[i];
    vec3 directionToLight = pointLight.position.xyz - fragPositionWorld;
    float attenuation = 1.0 / dot(directionToLight,directionToLight);
    float cosAngleIncidence = max(dot(surfaceNormal,normalize(directionToLight)),0);
    vec3 intensity = pointLight.color.xyz * pointLight.color.w * attenuation;
    diffuseLight += intensity * cosAngleIncidence; 
  }
  outColor = vec4(diffuseLight * fragColor,1.0); 

}