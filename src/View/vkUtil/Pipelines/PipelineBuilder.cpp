#include "View/vkUtil/Pipelines/PipelineBuilder.h"
#include <View/vkUtil/Pipelines/shaders.h>

vkInit::PipelineBuilder::PipelineBuilder(VkDevice device) {
	this->device = device;
	reset();

	//Some stages are fixed with sensible defaults and don't
	//need to be reconfigured
	configure_input_assembly();
	make_rasterizer_info();
	configure_multisampling();
	
	pipelineInfo.basePipelineHandle = nullptr;
}

vkInit::PipelineBuilder::~PipelineBuilder() {
	reset();
}

void vkInit::PipelineBuilder::reset() {
	pipelineInfo.flags = VkPipelineCreateFlags();

	reset_vertex_format();
	reset_shader_modules();
	reset_descriptor_set_layouts();

	pushConstantCount = 0;
	pushConstantInfo.offset = 0;
	pushConstantInfo.size = 0;
}

vkUtil::GraphicsPipelineOutBundle vkInit::PipelineBuilder::build(VkFormat imageFormat) {

	configure_color_blending();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	make_viewport_state();
	pipelineInfo.pViewportState = &viewportState;
	//Rasterizer
	pipelineInfo.pRasterizationState = &rasterizer;

	//Shader Modules
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();

	//Depth-Stencil is handled by depth attachment functions.

	//Multisampling
	pipelineInfo.pMultisampleState = &multisampling;

	//Color Blend
	pipelineInfo.pColorBlendState = &colorBlending;

	set_depth();
	//Pipeline Layout
	std::cout << "Create Pipeline Layout" << std::endl;
	VkPipelineLayout pipelineLayout;
	make_pipeline_layout(pipelineLayout);
	pipelineInfo.layout = pipelineLayout;



	VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info = {};
	pipeline_rendering_create_info.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	pipeline_rendering_create_info.colorAttachmentCount = 1;
	pipeline_rendering_create_info.pColorAttachmentFormats = &imageFormat;

	if (useDepthTest) {
		pipeline_rendering_create_info.depthAttachmentFormat = VkFormat::VK_FORMAT_D32_SFLOAT;  // Upewnij siê, ¿e format pasuje do twojej tekstury g³êbi
	}
	else {
		pipeline_rendering_create_info.depthAttachmentFormat = VkFormat::VK_FORMAT_UNDEFINED;
	}

	pipelineInfo.renderPass = nullptr;  // W dynamic rendering render pass jest niepotrzebny
	pipelineInfo.pNext = &pipeline_rendering_create_info;





	//Make the Pipeline
	std::cout << "Create Graphics Pipeline" << std::endl;

	VkPipeline graphicsPipeline;
	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);

	if (result != VK_SUCCESS) {
		std::cerr << "Failed to create Pipeline, error code: " << result << std::endl;
		return vkUtil::GraphicsPipelineOutBundle();
	}

	vkUtil::GraphicsPipelineOutBundle output;
	output.layout = pipelineLayout;
	output.pipeline = graphicsPipeline;


	return output;


	
}

void vkInit::PipelineBuilder::add_descriptor_set_layout(VkDescriptorSetLayout descriptorSetLayout) {
	descriptorSetLayouts.push_back(descriptorSetLayout);
}

void vkInit::PipelineBuilder::reset_descriptor_set_layouts() {
	descriptorSetLayouts.clear();
}

void vkInit::PipelineBuilder::specify_vertex_format(VkVertexInputBindingDescription bindingDescription, std::vector<VkVertexInputAttributeDescription> attributeDescriptions) {
	this->bindingDescription = bindingDescription;
	this->attributeDescriptions = attributeDescriptions;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &(this->bindingDescription);
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(this->attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = this->attributeDescriptions.data();
}

void vkInit::PipelineBuilder::specify_vertex_shader(const char* filename) {
	if (vertexShader) {
		vkDestroyShaderModule(device, vertexShader, nullptr);
		vertexShader = nullptr;
	}

	std::cout << "Create vertex shader module" << std::endl;
	vkUtil::createModule(filename, device,vertexShader);
	vertexShaderInfo = make_shader_info(vertexShader, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages.push_back(vertexShaderInfo);
}

void vkInit::PipelineBuilder::specify_fragment_shader(const char* filename) {
	if (fragmentShader) {
		vkDestroyShaderModule(device, fragmentShader, nullptr);
		fragmentShader = nullptr;
	}

	std::cout << "Create fragment shader module" << std::endl;
	vkUtil::createModule(filename, device, fragmentShader);
	fragmentShaderInfo = make_shader_info(fragmentShader, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
	shaderStages.push_back(fragmentShaderInfo);
}

void vkInit::PipelineBuilder::specify_swapchain_extent(VkExtent2D swapchainExtent) {
	this->swapchainExtent = swapchainExtent;
}

void vkInit::PipelineBuilder::clear_depth_attachment() {
	pipelineInfo.pDepthStencilState = nullptr;
}

void vkInit::PipelineBuilder::set_color_blending(bool is) {

	useColorBlending = is;
}

void vkInit::PipelineBuilder::use_depth_test(bool is) {
	this->useDepthTest = is;
}

void vkInit::PipelineBuilder::setPushConstants(VkShaderStageFlags stage,size_t size, int count) {
	pushConstantCount = count;
	pushConstantInfo.offset = 0;
	pushConstantInfo.size = size;
	pushConstantInfo.stageFlags = stage;
}

VkPipelineShaderStageCreateInfo vkInit::PipelineBuilder::make_shader_info(const VkShaderModule& shaderModule, const VkShaderStageFlagBits& stage) {
	VkPipelineShaderStageCreateInfo shaderInfo = {};
	shaderInfo.flags = VkPipelineShaderStageCreateFlags();
	shaderInfo.stage = stage;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";
	return shaderInfo;
}

VkPipelineViewportStateCreateInfo vkInit::PipelineBuilder::make_viewport_state() {

	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchainExtent.width;
	viewport.height = (float)swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset.x = 0.0f;
	scissor.offset.y = 0.0f;
	scissor.extent = swapchainExtent;

	viewportState.flags = VkPipelineViewportStateCreateFlags();
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	return viewportState;
}

void vkInit::PipelineBuilder::make_rasterizer_info() {
	rasterizer.flags = VkPipelineRasterizationStateCreateFlags();
	rasterizer.depthClampEnable = VK_FALSE; //discard out of bounds fragments, don't clamp them
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //This flag would disable fragment output
	rasterizer.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE; //Depth bias can be useful in shadow maps.
}

void vkInit::PipelineBuilder::configure_input_assembly() {
	inputAssemblyInfo.flags = VkPipelineInputAssemblyStateCreateFlags();
	inputAssemblyInfo.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

void vkInit::PipelineBuilder::configure_multisampling() {
	multisampling.flags = VkPipelineMultisampleStateCreateFlags();
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
}

void vkInit::PipelineBuilder::set_depth() {
	if (useDepthTest) {
		depthState.flags = VkPipelineDepthStencilStateCreateFlags();
		depthState.depthTestEnable = true;
		depthState.depthWriteEnable = true;
		depthState.depthCompareOp = VkCompareOp::VK_COMPARE_OP_LESS;
		depthState.depthBoundsTestEnable = false;
		depthState.stencilTestEnable = false;
		pipelineInfo.pDepthStencilState = &depthState;
	}
	else {
		depthState.flags = VkPipelineDepthStencilStateCreateFlags();
		depthState.depthTestEnable = false;
		depthState.depthWriteEnable = false;
		depthState.depthCompareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS;
		depthState.depthBoundsTestEnable = false;
		depthState.stencilTestEnable = false;
		pipelineInfo.pDepthStencilState = nullptr;
	}
}

void vkInit::PipelineBuilder::configure_color_blending() {
	colorBlendAttachment.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT;
	if (!useColorBlending) {
		colorBlendAttachment.blendEnable = VK_FALSE;
	}
	else {
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
	}


	colorBlending.flags = VkPipelineColorBlendStateCreateFlags();
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VkLogicOp::VK_LOGIC_OP_COPY;
	if (dynamicRendering) {
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
	}
	else {
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
	}

	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;
}

VkPipelineLayout vkInit::PipelineBuilder::make_pipeline_layout(VkPipelineLayout& pipelineLayout) {

	/*
	typedef struct VkPipelineLayoutCreateInfo {
		VkStructureType                 sType;
		const void*                     pNext;
		VkPipelineLayoutCreateFlags     flags;
		uint32_t                        setLayoutCount;
		const VkDescriptorSetLayout*    pSetLayouts;
		uint32_t                        pushConstantRangeCount;
		const VkPushConstantRange*      pPushConstantRanges;
	} VkPipelineLayoutCreateInfo;
	*/

	VkPipelineLayoutCreateInfo layoutInfo;
	layoutInfo.flags = VkPipelineLayoutCreateFlags();

	layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	layoutInfo.pSetLayouts = descriptorSetLayouts.data();



	layoutInfo.pushConstantRangeCount = pushConstantCount;
	layoutInfo.pPushConstantRanges = &pushConstantInfo;




	VkResult result = vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout);

	if (result != VK_SUCCESS) {
		std::cerr << "Failed to create pipeline layout! Error code: " << result << std::endl;
		return VK_NULL_HANDLE;
	}
}

void vkInit::PipelineBuilder::reset_vertex_format() {
	vertexInputInfo.flags = VkPipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;
}

void vkInit::PipelineBuilder::reset_shader_modules() {
	if (vertexShader) {
		vkDestroyShaderModule(device, vertexShader, nullptr);
		
	}
	if (fragmentShader) {
		vkDestroyShaderModule(device, fragmentShader, nullptr);
	}
	shaderStages.clear();
}

void vkInit::PipelineBuilder::set_overwrite_mode(bool mode) {
	overwrite = mode;
}
