#version 460
#extension GL_EXT_ray_tracing : enable

// ----------------- Uniformy i struktury -----------------

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;

struct RayPayload {
    vec3 color;
    float distance;
    vec3 normal;
    float reflector;
};

layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

struct ShadowPayload {
    // Zakładamy, że w shaderze cienia (dla cieni) 
    // w przypadku trafienia w przeszkodę ustawiamy isVisible na false.
    // Domyślnie można przyjąć, że światło jest widoczne (true)
    bool isVisible;
};

layout(location = 1) rayPayloadEXT ShadowPayload shadowPayload;

hitAttributeEXT vec2 attribs;

layout(set = 1, binding = 0) uniform sampler2DArray imageAtlas;

layout(binding = 3, set = 0) uniform LightSource {
    vec4 position;    // Pozycja punktowego światła
    vec4 intensity;   // Intensywność (kolor) światła
} light;

layout(binding = 4, set = 0) uniform Material {
    vec3 color;
    float shininess;
    float ambientCoefficient;
} material;

layout(set = 0, binding = 5) readonly buffer ExtraBLASData {
    vec4 data[];     // Dane geometrii (wierzchołki, normale, UV, tangenty)
} extraBLAS;

layout(set = 0, binding = 6) readonly buffer ExtraBLASOffsets {
    uint offsets[];  // Offsety do poszczególnych danych w ExtraBLAS
};

void isInShadow(vec3 point, vec3 lightPosition) {
    vec3 directionToLight = normalize(lightPosition - point);
    float tmin = 0.1f;  // Mały offset, aby zapobiec self-intersection
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

// ----------------- Funkcje PBR (Cook–Torrance) -----------------

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265 * denom * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Oblicza oświetlenie PBR z punktowym źródłem światła
vec3 calculatePBR(
    vec3 worldPos,
    vec3 N,
    vec3 V,
    vec3 albedo,
    float metallic,
    float roughness,
    vec3 radiance
) {
    vec3 L = normalize(light.position.xyz - worldPos);
    vec3 H = normalize(V + L);
    float distance = length(light.position.xyz - worldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 irradiance = radiance * attenuation;
    
    // BRDF Cook-Torrance
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - metallic);
    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / 3.14159265 + specular) * irradiance * NdotL;
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir, float dispIndex) { 
    float numLayers = 128.0;
    float parallaxScale = 0.5;
    float layerDepth = 1.0 / numLayers;
    
    vec2 deltaUV = (viewDir.xy * parallaxScale) / (viewDir.z * numLayers + 0.001);
    vec2 currUV = texCoords;
    
    float currDepth = 0.0;
    float depthValue = 1.0 - textureLod(imageAtlas, vec3(currUV, dispIndex), 0.0).a;
    
    for(float i = 0.0; i < numLayers; i++) {
        if(currDepth >= depthValue) break;
        currUV -= deltaUV;
        depthValue = 1.0 - textureLod(imageAtlas, vec3(currUV, dispIndex), 0.0).a;
        currDepth += layerDepth;
    }
    
    // Interpolacja między warstwami
    vec2 prevUV = currUV + deltaUV;
    float prevDepthValue = 1.0 - textureLod(imageAtlas, vec3(prevUV, dispIndex), 0.0).a;
    float prevDepth = currDepth - layerDepth;
    float weight = (currDepth - depthValue) / ((prevDepthValue - prevDepth) - (depthValue - currDepth));
    return mix(prevUV, currUV, clamp(weight, 0.0, 1.0));
}


// ----------------- Główny shader (closest hit) -----------------

void main() {
    uint instanceIndex = gl_InstanceCustomIndexEXT;
    // Interpolacja współrzędnych barycentrycznych
    const vec3 barycentricCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    uint primID = gl_PrimitiveID;
    
    // Pobieranie offsetów danych (indeksy, normale, UV, tangenty)
    uint indicesOffset   = offsets[5 * instanceIndex + 1];
    uint normalsOffset   = offsets[5 * instanceIndex + 2];
    uint texCoordsOffset = offsets[5 * instanceIndex + 3];
    uint tangentOffset   = offsets[5 * instanceIndex + 4];
    
    uint i0 = uint(extraBLAS.data[indicesOffset + primID].x);
    uint i1 = uint(extraBLAS.data[indicesOffset + primID].y);
    uint i2 = uint(extraBLAS.data[indicesOffset + primID].z);
    
    vec3 normal0 = extraBLAS.data[normalsOffset + i0].xyz;
    vec3 normal1 = extraBLAS.data[normalsOffset + i1].xyz;
    vec3 normal2 = extraBLAS.data[normalsOffset + i2].xyz;
    
    vec2 uv0 = extraBLAS.data[texCoordsOffset + i0].xy;
    vec2 uv1 = extraBLAS.data[texCoordsOffset + i1].xy;
    vec2 uv2 = extraBLAS.data[texCoordsOffset + i2].xy;
    
    vec3 tangent0 = extraBLAS.data[tangentOffset + i0].xyz;
    vec3 tangent1 = extraBLAS.data[tangentOffset + i1].xyz;
    vec3 tangent2 = extraBLAS.data[tangentOffset + i2].xyz;
    
    // Parametry tekstury (indeksy do atlasu):
    // x – diffuse, y – normal map, z – ARM, w – displacement
    vec4 textureIndices = vec4(
        extraBLAS.data[texCoordsOffset + i0].zw,
        extraBLAS.data[normalsOffset + i0].w,
        extraBLAS.data[indicesOffset + primID].w
    );
    
    vec2 uv = uv0 * barycentricCoords.x + uv1 * barycentricCoords.y + uv2 * barycentricCoords.z;
    vec3 normalInterp = normalize(normal0 * barycentricCoords.x + normal1 * barycentricCoords.y + normal2 * barycentricCoords.z);
    vec3 tangentInterp = normalize(tangent0 * barycentricCoords.x + tangent1 * barycentricCoords.y + tangent2 * barycentricCoords.z);
    
    vec3 worldPos = gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT;
    

    vec3 T = tangentInterp;
    vec3 N = normalInterp;
    T = normalize(T - dot(T, N) * N); // ortogonalizacja
	//float handedness = -1.0f;
    vec3 B =   cross(N, T);
    mat3 TBN = mat3(T, B, N);
    
    
    
    if(textureIndices.y > 0.1f) {
        vec3 sampledNormal = texture(imageAtlas, vec3(uv, textureIndices.y)).rgb;
        sampledNormal = normalize(sampledNormal * 2.0 - 1.0); // przekształcenie z [0,1] do [-1,1]
        N = normalize(TBN * sampledNormal);
    }
    vec3 viewDir = normalize(gl_WorldRayOriginEXT - worldPos);
    vec3 viewDirTangent = transpose(TBN) * viewDir;
    uv = ParallaxMapping(uv, viewDirTangent, textureIndices.w);
    
    vec3 albedo = texture(imageAtlas, vec3(uv, textureIndices.x)).rgb;
    vec3 loadARM = texture(imageAtlas, vec3(uv, textureIndices.z)).rgb;
    float ao = loadARM.r;
    float roughness = loadARM.g;
    float metallic = loadARM.b;
    
    float shadow = 1.0;
    isInShadow(worldPos, light.position.xyz);
    if(shadowPayload.isVisible) {
        shadow = 0.0;
    }
	else{
	
	}
    
    vec3 radiance = light.intensity.xyz;
    vec3 pbrColor = calculatePBR(worldPos, N, viewDir, albedo, metallic, roughness, radiance);
    
    vec3 ambient = material.ambientCoefficient * albedo * ao;
    
    vec3 finalColor = ambient + (pbrColor * shadow);
    
    if((albedo.r == 1.0) && (albedo.g == 1.0) && (albedo.b == 1.0))
        rayPayload.reflector = 1.0;
    else if((albedo.r < 0.01) && (albedo.g < 0.01) && (albedo.b < 0.01)) {
        rayPayload.reflector = 0.5;
        finalColor = vec3(1.0);
    }
    else
        rayPayload.reflector = 0.0;
    
    rayPayload.color = finalColor;
    rayPayload.distance = gl_RayTmaxEXT;
    rayPayload.normal = N;
}
