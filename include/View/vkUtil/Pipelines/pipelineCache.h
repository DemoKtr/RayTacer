#pragma once
#include "config.h"
#include "View/vkUtil/Pipelines/PipelineBuilder.h"
#include <unordered_map>

namespace vkUtil {

    struct PipelineCacheChunk {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        
        // 0 - rendering scene
        // 1 - postprocess
    };

    class PipelineCache {
    private:
        std::unordered_map<std::string, PipelineCacheChunk> pipelines;
        VkDevice device;

    public:
        PipelineCache(VkDevice device);

        vkUtil::PipelineCacheChunk getPipeline(const std::string& name);

        void addPipeline(const std::string& name, const PipelineCacheChunk& pipeline);

        ~PipelineCache();
    };


}