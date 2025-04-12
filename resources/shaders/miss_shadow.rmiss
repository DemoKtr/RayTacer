#version 460
#extension GL_EXT_ray_tracing : require

struct ShadowPayload {
    bool isVisible;
};

layout(location = 0) rayPayloadInEXT ShadowPayload shadowPayload;

void main() {
    shadowPayload.isVisible = true;
}
