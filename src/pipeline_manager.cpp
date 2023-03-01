#include "pipeline_manager.h"

void PipelineManager::CreateGraphicsPipeline()
{
    std::vector<char> vert_shader_code = SimpleFileReader::ReadFile("../shaders/vert.spv");
    std::vector<char> frag_shader_code = SimpleFileReader::ReadFile("../shaders/frag.spv");

    VkShaderModule vert_shader_module = CreateShaderModule(vert_shader_code);
    VkShaderModule frag_shader_module = CreateShaderModule(frag_shader_code);

    VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module;

    // specifying the entrypoint (the function inside the shader which will be invoked)
    // this makes it possible to combine multiple shaders (book says fragment shaders only) into a single shader module and
    //  use different entry points to differentiate between their behaviours
    vert_shader_stage_info.pName = "main";

    // there is a another struct value that is ignored here called " pSpecializationInfo"
    // this value allows passing in data for shader constants
    // this way, the shader's behavior can be configured at pipeline creation

    VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

    // Most of the pipeline needs to be baked in, a limited about  of the state can be changed without recreating the pipeline at draw time
    // Example : size of viewport, line width and blend constants

    // Vertex Input
    // Describing the format of the vertex data that will be passed into the vertex shader:
    //  - Bindings : spacing between data and whether the data is per-vertex or per-instance
    //  - Attribute descriptions : type of the attributes passed to the vertex shader, which binding to load them from and which offset

    // pVertexBindingDescriptions and pVertexAttributeDescriptions point to array of structs

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    // Input Assembly
    // describes what kind of geometry will be drawn from the vertices (point list, line list, line strip, triangle list, triangle strip)
    // and if primitive restart enable should be enabled
    //  generally vertices are loaded in sequential manner but with an element buffer, vertex indices can be specified. This allows
    // optimizations like reusing vertices
    //  if primitiveRestartEnable is set to true, it allows to break up line/triangle strips by specifying a special index (0xFFFFFFF)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport
    // Viewport describes the region of the framebuffer that the output will be rendered to
    // this performs a scale transformation if the size of the viewport is smaller than the size of the framebuffer
    // This will almost always be (0,0) to (width, height)
    VkViewport viewport{};
    VkExtent2D swapchain_extent = SwapchainManager_->GetSwapchainExtent();
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchain_extent.width;
    viewport.height = (float)swapchain_extent.height;
    viewport.minDepth = 0.0f; // min/maxDepth values specify the range of depth values for the framebuffer
    viewport.maxDepth = 1.0f;

    // Scissor
    // The scissor rectangle defines in which region pixels will actually be stored. Pixels outside this rectangle will be discarded by rasterizer
    // This acts like a filter rather than a transformation
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain_extent;

    // Viewport and scissor can be specified as a static part of the pipeline or as a dynamic state set in the command buffer
    // Its convenient to make the scissor and viewport state dynamic since it gives a lot of flexibility. This is very common.
    // If opting for dynamic viewport and scissor, need to enable the respective dynamic state for the pipeline
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    //Rasterizer
    //Takes geometry shaped by vertices from the vertex shader and turns it into fragments to be colored by fragment shader
    //Performs  DEPTH TESTING  ,  FACE CULLING  and the  SCISSOR TEST 
    //Can be configured to output fragments that fill entire polygons or just the edges (wireframe rendering)
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    //if depthClampEnable is true, fragments beyond the near and far planes are clamped to them instead of discarding them
    //useful in special cases like shadow maps
    //using this requires a GPU feature
    rasterizer.depthClampEnable = VK_FALSE;
    //if rasterizerDiscardEnables is true, geometry never passes to the rasterizer stage. This disables any output to the framebuffer
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    //polygonMode determines how fragments are generated. Available modes are fill, line, point (using mode other than fill requires GPU feature)
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    //linewidth describes thickness of lines in terms of number of fragments. (any line thicker than 1 requires GPU feature)
    rasterizer.lineWidth = 1.0f;
    //cullmode determines the type of face culling. This can be to cull front faces, cull back faces or both)
    //frontFace specifies the vertex order for faces to be considered front-facing (clockwise or anti-clockwise)
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    //rasterizer can alter depth values by adding a constant value or biasing them based on a fragment's slope (sometimes used for shadow mapping)
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    //Multisampling
    //One of the ways to perform anti-aliasing. Works by combining fragment shader results of multiple polygons that rasterize to the same pixel
    //Enabling requires a GPU feature
    VkPipelineMultisampleStateCreateInfo multisampling;
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    //Depth and stencil testing
    //When using create and initialize the struct VkPipelineDepthStencilStateCreateInfo

    //Color Blending
    //After a fragment shader returns a color, it needs to be combined with the color that is already in the framebuffer
    //There are two ways of doing this : 
    // - mix old and new value to produce a final color
    // - combine the old and new values using the bitwise operation
    //There are also two structs to configure color blending:
    // - VkPipelineColorBlendAttachmentState : configuration per attached framebuffer
    // - VkPipelineColorBlendStateCreateInfo : contains global color blending setting
    //for now, there's only one framebuffer
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    // check https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions#page_Color-blending 
    //      for explanation on how color blending works

    VkPipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE; //enables second method of blending (bitwise combination)
    colorBlending.logicOp = VK_LOGIC_OP_COPY; //the bitwise operation can be specified here (this will disable the first metho, as if blendEnable = VK_FALSE for all framebuffers)
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    //colorWriteMask of previous struct will be used here to determine which channels should be affected


    //Pipeline Layout
    //For uniform values in shaders, which are globals similar to dynamic state variables that can be changed at 
    // drawing time to alter behavior of shaders without having to recreate them
    //Commonly used to pass tranformation matrix to the vertex shader, or to create texture samplers in the fragment shader
    // PipelineLayout_ has already been declared

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;



    if(vkCreatePipelineLayout(LogicalDevice_, &pipelineLayoutInfo, nullptr, &PipelineLayout_) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    CreateRenderPass();

    //To create the final pipeline object, we need:
    //  - shader stages
    //  - fixed function state
    //  - pipeline layout
    //  - render pass

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    //shader stages
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shader_stages;
    
    //fixed function state
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pColorBlendState =&colorBlending;
    pipelineInfo.pDynamicState = nullptr;

    //pipeline layout
    pipelineInfo.layout = PipelineLayout_;
    
    //render pass
    pipelineInfo.renderPass = RenderPass_;
    pipelineInfo.subpass = 0;

    //Vulkan allows creating a new pipeline object based on an existing pipeline by deriving from it
    // This can be specified in the following fields
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if(vkCreateGraphicsPipelines(LogicalDevice_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &Pipeline_) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Pipeline");
    }


    vkDestroyShaderModule(LogicalDevice_, vert_shader_module, nullptr);
    vkDestroyShaderModule(LogicalDevice_, frag_shader_module, nullptr);
}

void PipelineManager::CreateRenderPass()
{
    //Before creating the pipeline, we need to tell Vulkan about the framebuffer attachments that will be used for rendering
    //Need to specify:
    // - how many color and depth buffers there will be
    // - how many samples to use for each
    // - how their contents should be handled through the rendering operations
    //These details are wrapped in a renderpass object

    //format of the color attachment should match the swapchain
    //no multisampling hence samples = 1
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = SwapchainManager_->GetSwapchainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    
    //loadOp and storeOp determine what to do with the data in the attachment before rendering and after rendering
    // - loadOp : LOAD (preserve existing values), CLEAR (clear values to a constant), DONT_CARE (dont care)
    // - storeOp : STORE (rendered contents will be stored in memory and can be read later), DONE CARE (contents of framebuffer underfined after rendering operation)
    // loadOp and storeOp apply to color and depth data (stencilLoadOp/stencilStoreOp apply to stencil data)
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    //Textures and framebuffers in Vulkan are represented by VkImage objects with certain pixel format
    // However, the layout of the pixels in memory can change based on what you're trying to do with the image
    // Layout can specify to use image as color attachment, present images to the swapchain, used as destination for memory transfer operation
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //dont care what the previous layout of the image was
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //we want image to be ready for presentation using swapchain after rendering


    //Subpasses
    //A single render pass can contain multiple subpasses
    //Supasses are subsequent rendering operations that depend on the contents of the framebuffer in the previous passes
    
    //Every subpass references one or more attachments that we have described
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; //specifies attachment to reference by using its index in the attachment descriptions array (part of
    //                                      the VkRenderPassCreateInfo struct which will be defined later)
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //describes the layout for this attachment during this subpass

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //specifying that this is a graphics subpass (instead of compute)
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; //specify reference to the color attachment

    // NOTE : Fragment Shader *layout* keyword also uses the same index as the index of the attachment in the attachment descriptions array
    //      layout(location = 0) out vec4 outColor

    /*
    There are other types of attachments that can be referenced by a subpass:
        1. pInputAttachments : attachments that are read from a shader
        2. pResolveAttachments : attachments used for multisampling color attachments
        3. pDepthStencilAttachment : attachment for depth and stencil data
        4. pPreserveAttachments : attachments not used by this subpass but for which data must be preserved
    */

    //Finally, create the render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment; //attachment description array
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if(vkCreateRenderPass(LogicalDevice_, &renderPassInfo, nullptr, &RenderPass_) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }

}

void PipelineManager::DestroyPipelineLayout()
{
    vkDestroyPipelineLayout(LogicalDevice_, PipelineLayout_, nullptr);
}

void PipelineManager::DestroyRenderPass()
{
    vkDestroyRenderPass(LogicalDevice_, RenderPass_, nullptr);
}

void PipelineManager::DestroyPipeline()
{
    vkDestroyPipeline(LogicalDevice_, Pipeline_, nullptr);
    DestroyPipelineLayout();
}

VkShaderModule PipelineManager::CreateShaderModule(const std::vector<char> &bytecode)
{
    VkShaderModuleCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = VK_NULL_HANDLE;
    createInfo.flags = 0;
    createInfo.codeSize = static_cast<uint32_t>(bytecode.size());

    // bytecode pointer needs to be uint32_t* according to Vulkan specification, hence we need reinterpret_cast
    // when using reinterpret cast like this, it must be kept in mind that the data must satisfy the alignement requirements for uint32_t
    // here, the std::vector default allocator
    createInfo.pCode = reinterpret_cast<const uint32_t *>(bytecode.data());

    VkShaderModule shader_module;

    if (vkCreateShaderModule(LogicalDevice_, &createInfo, nullptr, &shader_module) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shader_module;
}
