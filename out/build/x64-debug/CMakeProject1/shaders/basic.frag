#version 410 core

in vec4 FragPos;
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform vec3 viewPos;
//matrices
uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;
uniform mat3 normalMatrix;


//lighting
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float far_plane;
uniform samplerCube depthMap;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
 
//point light struct
struct PointLight {    
    vec3 position;
    vec3 color;

    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
#define NR_POINT_LIGHTS 4 
uniform PointLight pointLights[4];
vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   
float ShadowCalculation(vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
    float shadow = 0.0;
    float bias   = 0.15;
    int samples  = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;  
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
    return shadow;
}  
vec3 computeGlobalLight(vec3 fragPos,vec3 normalEye ,vec3 viewDir)
{

	vec3 color = texture(diffuseTexture, fTexCoords).rgb;
    vec3 normal = normalEye;
    vec3 lightColor = vec3(1);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
   
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fragPos);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;   
	vec3 directional = (ambient + (1.0 - shadow) * (diffuse + specular));
	return directional;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
    float dist    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + 
  			     light.quadratic * (dist * dist));    
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(diffuseTexture, fTexCoords)) * light.color;
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(diffuseTexture, fTexCoords)) * light.color;
    vec3 specular = light.specular * spec * vec3(texture(specularTexture, fTexCoords)) * light.color;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

void main() 
{
    vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
    if(colorFromTexture.a < 0.1)
	    discard;

    vec3 fragPos = (model * vec4(fPosition,1.0f)).xyz;
	vec3 color = texture(diffuseTexture, fTexCoords).rgb;
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 viewDir = normalize(viewPos -  FragPos.xyz);
    //vec3(normalEye * FragPos.xyz,1.0f))
	vec3 lighting = computeGlobalLight(fragPos,normalEye,viewDir);// vec3(0,0,0);//  computeDirLight(normalEye,viewDir);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        lighting += CalcPointLight(pointLights[i], normalEye, fragPos, viewDir)/  color;
       
    }
    

    FragColor = vec4(lighting *  color, 1.0f);
     float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
