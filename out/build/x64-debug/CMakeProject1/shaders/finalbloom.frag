#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform bool gamma;
uniform float exposure;


void main()
{      
    vec3 hdrColor = texture(scene, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
     if(bloom)
            hdrColor += bloomColor; // additive blending
    if(gamma)
    {
        const float gamma = 2.2;
        // tone mapping
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
        // also gamma correct while we're at it       
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
    else
    {
        FragColor = vec4(hdrColor, 1.0);
    }

   
}