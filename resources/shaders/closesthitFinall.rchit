#version 460
#extension GL_EXT_ray_tracing : enable

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;


struct RayPayload {
	vec3 color;
	float distance;
	vec3 normal;
	float reflector;
};

layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

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


void isInShadow(vec3 point, vec3 lightPosition) {
    vec3 directionToLight = normalize(lightPosition - point);
    float tmin = 0.1f;  // Small offset to avoid self-intersection
    float tmax = length(lightPosition - point);
	shadowPayload.isVisible = true;
    traceRayEXT(
    topLevelAS,
    gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT, 
	0xFF, 
	0, 
	0, 
	1,
    point, tmin, directionToLight, tmax, 1
);

	
    
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

    vec3 lightDir = normalize(light.position.xyz - worldPos);
    vec3 viewDir = normalize(gl_WorldRayOriginEXT - worldPos);

    vec3 objectColor = texture(imageAtlas, texCoord).rgb;
	
	vec3 lightVector = normalize(light.position.xyz);
	float dot_product = max(dot(lightVector, normal), 0.6);
	rayPayload.color = objectColor * vec3(dot_product);
	rayPayload.distance = gl_RayTmaxEXT;
	rayPayload.normal = normal;

	// Objects with full white vertex color are treated as reflectors
	if((objectColor.r == 1.0f) && (objectColor.g == 1.0f) && (objectColor.b == 1.0f))
	rayPayload.reflector = 1.0f;
	else if((objectColor.r < .3f) && (objectColor.g < .3f) && (objectColor.b < .3f)) {
	rayPayload.reflector = 0.5f;
	rayPayload.color = vec3(1.0f,1.0f,1.0f);
	}
	else rayPayload.reflector = .0f;


	
}
