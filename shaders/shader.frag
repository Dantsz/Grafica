#version 410 core

uniform vec3 baseColor;

in vec3 color;



out vec4 fColor;
in vec3 v_normal;
in vec3 v_position;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform vec3 lightDir;
uniform vec3 lightColor;


vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
void computeLight()
{
	//compute eye space coordinates
	vec4 vertPosEye = view * model * vec4(v_position, 1.0f);
	vec3 normalEye = normalize(normalMatrix * v_normal);
	
	//normalize light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction (in eye coordinates, the viewer is situated at the origin
	vec3 viewDir = normalize(- vertPosEye.xyz);
	
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflectDir = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
	specular = specularStrength * specCoeff * lightColor;
}
void main()
{	
	computeLight();
	fColor = vec4(min((ambient + diffuse) * baseColor + specular, 1.0f),1.0f);
}