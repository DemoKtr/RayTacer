#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec2 attribs;

layout(set=1,binding=0) uniform sampler2DArray imageAtlas;

layout(binding = 3, set = 0) uniform LightSource {
    vec4 position;
    vec4 intensity;
} light;

layout(binding = 4, set = 0) uniform Material {
    vec3 color;
    float shininess;
    float ambientCoefficient;
} material;

layout(set = 0, binding = 5) readonly buffer ExtraBLASData {
    float data[];
} extraBLAS;

layout(set = 0, binding = 6) readonly buffer ExtraBLASOffsets {
    uint offsets[];
};

vec3 CalculatePhongLighting(vec3 lightDir, vec4 lightIntensity,
                            vec3 normal, vec3 viewDir,
                            vec3 objectColor, float shininess,
                            float ambientCoefficient);

void main() {
    uint instanceIndex = gl_InstanceCustomIndexEXT;
    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    uint primID = gl_PrimitiveID;

    uint indicesOffset = offsets[3 * instanceIndex + 0];
    uint normalsOffset = offsets[3 * instanceIndex + 1];
    uint texCoordsOffset = offsets[3 * instanceIndex + 2];

    uint i0 = uint(extraBLAS.data[indicesOffset + 3 * primID]);
    uint i1 = uint(extraBLAS.data[indicesOffset + 3 * primID + 1]);
    uint i2 = uint(extraBLAS.data[indicesOffset + 3 * primID + 2]);

    vec3 normal0 = vec3(extraBLAS.data[normalsOffset + 3 * i0], extraBLAS.data[normalsOffset + 3 * i0 + 1], extraBLAS.data[normalsOffset + 3 * i0 + 2]);
    vec3 normal1 = vec3(extraBLAS.data[normalsOffset + 3 * i1], extraBLAS.data[normalsOffset + 3 * i1 + 1], extraBLAS.data[normalsOffset + 3 * i1 + 2]);
    vec3 normal2 = vec3(extraBLAS.data[normalsOffset + 3 * i2], extraBLAS.data[normalsOffset + 3 * i2 + 1], extraBLAS.data[normalsOffset + 3 * i2 + 2]);

    vec2 texCoord0 = vec2(extraBLAS.data[texCoordsOffset + 2 * i0], extraBLAS.data[texCoordsOffset + 2 * i0 + 1]);
    vec2 texCoord1 = vec2(extraBLAS.data[texCoordsOffset + 2 * i1], extraBLAS.data[texCoordsOffset + 2 * i1 + 1]);
    vec2 texCoord2 = vec2(extraBLAS.data[texCoordsOffset + 2 * i2], extraBLAS.data[texCoordsOffset + 2 * i2 + 1]);

    // interpolate normals and texcoords based on barycentric coordinates
    vec3 normal = normalize(normal0 * barycentricCoords.x + normal1 * barycentricCoords.y + normal2 * barycentricCoords.z);
vec3 texCoord = vec3(texCoord0 * barycentricCoords.x + texCoord1 * barycentricCoords.y + texCoord2 * barycentricCoords.z, 0.0);
    vec3 lightDir = normalize(vec3(light.position) - gl_WorldRayOriginEXT.xyz);
    vec3 viewDir = normalize(gl_WorldRayOriginEXT.xyz - gl_WorldRayDirectionEXT.xyz);

    vec3 objectColor = texture(imageAtlas, texCoord).rgb;

    vec3 finalColor = CalculatePhongLighting(
        lightDir,
        light.intensity,
        normal,
        viewDir,
        objectColor,
        material.shininess,
        material.ambientCoefficient
    );

    hitValue = finalColor;
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