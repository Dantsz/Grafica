#version 410 core

in vec4 FragPos;
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
out vec4 fColor;

uniform vec3 viewPos;
//matrices
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;
vec4 fPosEye = view * model * vec4(fPosition, 1.0f);

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
 
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

float computeShadow()
{	
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;
	// Check whether current frag pos is in shadow
	float bias = max(0.05f * (1.0f - dot(fNormal, lightDir)), 0.005f);
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
	return shadow;
}
vec3 computeDirLight(vec3 normalEye ,vec3 viewDir)
{
	float ambientStrength = 0.1f;
	float specularStrength = 0.25f;
	float shininess = 1.0f;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;


    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
   

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;
	float shadow = computeShadow();
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
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
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

    vec3 viewDir = normalize(viewPos - fPosition.xyz);
    vec3 norm = normalize(fNormal);
	vec3 color = texture(diffuseTexture, fTexCoords).rgb;
    vec3 normalEye = normalize(normalMatrix * fNormal);
     
	vec3 lighting = computeDirLight(normalEye,viewDir); vec3(0,0,0);//  computeDirLight(normalEye,viewDir);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        color += CalcPointLight(pointLights[i], normalEye, FragPos.xyz, viewDir); 
    }

	

    fColor = vec4(lighting *  color, 1.0f);
}
