#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 baseColor;

out vec3 color;
out vec3 v_normal;
out vec3 v_position;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;


void main()
{	
	v_normal = vNormal;
	v_position = vPosition;
	color = baseColor ;


	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
} 
