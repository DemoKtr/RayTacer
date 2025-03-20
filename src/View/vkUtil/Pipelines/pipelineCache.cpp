#include "View/vkUtil/Pipelines/pipelineCache.h"

vkUtil::PipelineCache::PipelineCache(VkDevice device) : device(device) {

}

vkUtil::PipelineCacheChunk vkUtil::PipelineCache::getPipeline(const std::string& name) {
    auto it = pipelines.find(name);
    if (it != pipelines.end()) {
        return it->second;
    }
    throw std::runtime_error("Pipeline with name '" + name + "' not found.");
}

void vkUtil::PipelineCache::addPipeline(const std::string& name, const PipelineCacheChunk& pipeline) {
    if (pipelines.find(name) != pipelines.end()) {
        throw std::runtime_error("Pipeline with name '" + name + "' already exists.");
    }

    pipelines[name] = pipeline;
}

vkUtil::PipelineCache::~PipelineCache() {

    for (auto& pair : pipelines) {
        vkDestroyPipeline(device, pair.second.pipeline, nullptr);
        vkDestroyPipelineLayout(device, pair.second.pipelineLayout, nullptr);
    }

}
