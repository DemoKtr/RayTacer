#version 460
#extension GL_EXT_ray_tracing : enable

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;

layout(location = 0) rayPayloadInEXT vec3 hitValue;

struct ShadowPayload {
    bool isVisible;
};

layout(location = 1) rayPayloadEXT ShadowPayload shadowPayload;
hitAttributeEXT vec2 attribs;

layout(set=1,binding=0) uniform sampler2DArray imageAtlas;

layout(binding = 3, set = 0) uniform LightSource {
    vec4 position;    // Light source position
    vec4 intensity;   // Light intensity (color)
} light;

layout(binding = 4, set = 0) uniform Material {
    vec3 color;
    float shininess;
    float ambientCoefficient;
} material;

layout(set = 0, binding = 5) readonly buffer ExtraBLASData {
    vec4 data[];     // Geometry data
} extraBLAS;

layout(set = 0, binding = 6) readonly buffer ExtraBLASOffsets {
    uint offsets[];  // Offsets to different data in ExtraBLAS
};

vec3 CalculatePhongLighting(vec3 lightDir, vec4 lightIntensity,
                            vec3 normal, vec3 viewDir,
                            vec3 objectColor, float shininess,
                            float ambientCoefficient);

// Shadow checking function
bool isInShadow(vec3 point, vec3 lightPosition) {
    vec3 directionToLight = normalize(lightPosition - point);
    float tmin = 0.001;  // Small offset to avoid self-intersection
    float tmax = length(lightPosition - point);

    // Perform ray tracing towards the light
    shadowPayload.isVisible = false;
    traceRayEXT(topLevelAS,  gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipClosestHitShaderEXT, 0xff, 1, 2, 1, point, tmin, directionToLight, tmax, 1);  // Use ray flag to detect shadow ray


    // If we hit something, we are in shadow
    return shadowPayload.isVisible;
}

void main() {
    uint instanceIndex = gl_InstanceCustomIndexEXT;
    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    uint primID = gl_PrimitiveID;

    uint indicesOffset = offsets[4 * instanceIndex + 1];
    uint normalsOffset = offsets[4 * instanceIndex + 2];
    uint texCoordsOffset = offsets[4 * instanceIndex + 3];

    uint i0 = uint(extraBLAS.data[indicesOffset + primID].x);
    uint i1 = uint(extraBLAS.data[indicesOffset + primID].y);
    uint i2 = uint(extraBLAS.data[indicesOffset + primID].z);

    vec3 normal0 = extraBLAS.data[normalsOffset + i0].xyz;
    vec3 normal1 = extraBLAS.data[normalsOffset + i1].xyz;
    vec3 normal2 = extraBLAS.data[normalsOffset + i2].xyz;

    vec2 texCoord0 = extraBLAS.data[texCoordsOffset + i0].xy;
    vec2 texCoord1 = extraBLAS.data[texCoordsOffset + i1].xy;
    vec2 texCoord2 = extraBLAS.data[texCoordsOffset + i2].xy;

    vec3 normal = normalize(normal0 * barycentricCoords.x + normal1 * barycentricCoords.y + normal2 * barycentricCoords.z);
    vec3 texCoord = vec3(texCoord0 * barycentricCoords.x + texCoord1 * barycentricCoords.y + texCoord2 * barycentricCoords.z, 0.0);
    vec3 worldPos = gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT;

    vec3 lightDir = normalize(vec3(light.position) - worldPos);
    vec3 viewDir = normalize(gl_WorldRayOriginEXT.xyz - gl_WorldRayDirectionEXT.xyz);

    vec3 objectColor = texture(imageAtlas, texCoord).rgb;

    // Check if the hit point is in shadow
    bool shadowed = isInShadow(worldPos, light.position.xyz);

    // If the point is shadowed, reduce the light intensity to zero (darkened)
    vec3 finalColor = CalculatePhongLighting(
        lightDir,
        shadowed ? vec4(0.0) : light.intensity,
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
