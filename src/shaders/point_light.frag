#version 450

layout (location = 0 ) in vec2 fragOffset;
layout (location = 0 ) out vec4 outColor;

struct PointLight{
  vec4 position;
  vec4 color;
};
layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec4 ambientLightColor;
  PointLight pointlights[1000];
  int numLights;

} ubo;
layout(push_constant) uniform Push{
  vec4 position;
  vec4 color;
  float radius;


} push;
const float M_PI = 3.1415926536;
void main(){
    float dis = sqrt(dot(fragOffset,fragOffset));
    if(dis >= 1.0){
        discard;
    }
    float cosineDist = cos(dis * M_PI);
    float colorEquation = 1- (cos(dis * M_PI) * sin(dis * M_PI));
   outColor = vec4(push.color.xyz * (0.4 * (colorEquation + 0.4) ) ,0.5 *( cosineDist + 1.0));
}