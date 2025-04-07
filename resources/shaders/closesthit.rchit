#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;

layout(binding = 3, set = 0) uniform LightSource
{
    vec4 position;
    vec4 intensity;
} light; 
layout(binding = 4, set = 0) uniform Material
{
    vec3 color;
    float shininess;
    float ambientCoefficient;
} material; 

// Assuming there is an implementation of the Phong Lighting Shader
vec3 CalculatePhongLighting(vec3 lightDir, vec4 lightIntensity,
                            vec3 normal, vec3 viewDir, 
                            vec3 objectColor, float shininess, 
                            float ambientCoefficient);

void main() 
{
    vec3 lightDir = normalize(vec3(light.position.x,light.position.y,light.position.z) - gl_WorldRayOriginEXT);
    vec3 normal = vec3(1);
    vec3 viewDir = normalize(gl_WorldRayOriginEXT - gl_WorldRayDirectionEXT);

    vec3 color = CalculatePhongLighting(lightDir, light.intensity, normal, viewDir, material.color, material.shininess, material.ambientCoefficient);

    hitValue = color;
}

vec3 CalculatePhongLighting(vec3 lightDir, vec4 lightIntensity,
                            vec3 normal, vec3 viewDir, 
                            vec3 objectColor, float shininess, 
                            float ambientCoefficient) 
{
    float diffuseFactor = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);  
    float specFactor = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient = ambientCoefficient * objectColor;
    vec3 diffuse = diffuseFactor * objectColor;
    vec3 specular = specFactor * vec3(lightIntensity);

    return ambient + diffuse + specular;
}