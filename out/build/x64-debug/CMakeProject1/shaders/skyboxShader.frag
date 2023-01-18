#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform float iTime;
#define pi 3.14159
struct Wave{
    vec2 pos; // wave source position
    float T; // period
    float lambda; // wavelength
};
float getVal(Wave w, vec2 uv){
    return sin(2.*pi*(iTime/w.T - length(uv-w.pos)/w.lambda));
}
void main()
{
    Wave w1 = Wave(vec2(.3,.3),.9,.05);
    color = texture(skybox, textureCoordinates);
    float val = (2.+getVal(w1,textureCoordinates.xz));
    if(textureCoordinates.y < -0.5)
    {
    color *= vec4(val*vec3(1),1.0);
    }
    
}
