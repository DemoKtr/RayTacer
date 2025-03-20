#pragma once
#include "config.h"
#include "View/vkUtil/Pipelines/pipelineStructs.h"
#include <vector>
#include <unordered_map>
namespace vkInit {

	class PipelineBuilder {
	public:
		bool dynamicRendering = false;
		PipelineBuilder(VkDevice device);
		~PipelineBuilder();

		void reset();

		vkUtil::GraphicsPipelineOutBundle build(VkFormat imageFormat);
		void add_descriptor_set_layout(VkDescriptorSetLayout descriptorSetLayout);
		void reset_descriptor_set_layouts();
		

		void specify_vertex_format(
			VkVertexInputBindingDescription bindingDescription,
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions);

		void specify_vertex_shader(const char* filename);

		void specify_fragment_shader(const char* filename);
		void specify_swapchain_extent(VkExtent2D swapchainExtent);
		void clear_depth_attachment();
		void set_color_blending(bool is);
		void use_depth_test(bool is);

		void setPushConstants(VkShaderStageFlags stage, size_t size, int count);

		void set_overwrite_mode(bool mode);
	private:
		VkDevice device;
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		VkPushConstantRange pushConstantInfo;
		int pushConstantCount = 0;
		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};

		VkPipelineShaderStageCreateInfo vertexShaderInfo, fragmentShaderInfo;
		
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		//VkShaderModule vertexShader = nullptr, fragmentShader = nullptr;
		//VkPipelineShaderStageCreateInfo vertexShaderInfo, fragmentShaderInfo;

		VkExtent2D swapchainExtent;
		VkViewport viewport = {};
		VkRect2D scissor = {};
		VkPipelineViewportStateCreateInfo viewportState = {};

		VkPipelineRasterizationStateCreateInfo rasterizer = {};

		VkPipelineDepthStencilStateCreateInfo depthState;
		std::unordered_map<uint32_t, VkAttachmentDescription> attachmentDescriptions;
		std::unordered_map<uint32_t, VkAttachmentReference> attachmentReferences;
		std::vector<VkAttachmentDescription> flattenedAttachmentDescriptions;
		std::vector<VkAttachmentReference> flattenedAttachmentReferences;

		VkPipelineMultisampleStateCreateInfo multisampling = {};

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		VkPipelineColorBlendStateCreateInfo colorBlending = {};

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		VkShaderModule vertexShader = nullptr, fragmentShader = nullptr;
		
		bool overwrite, useDepthTest, useColorBlending;

		VkPipelineShaderStageCreateInfo make_shader_info(
			const VkShaderModule& shaderModule, const VkShaderStageFlagBits& stage);


		VkPipelineViewportStateCreateInfo make_viewport_state();
		void make_rasterizer_info();
		void configure_input_assembly();

		void configure_multisampling();

		void set_depth();
		void configure_color_blending();

		VkPipelineLayout make_pipeline_layout(VkPipelineLayout& pipelineLayout);


		void reset_vertex_format();
		void reset_shader_modules();
		
	};

}