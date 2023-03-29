#include "ivr_app.h"


void IvrApp::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void IvrApp::initWindow() 
{
    glfwInit(); //initializes the glfw library
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //glfw was originally designed to create an OpenGL context. This function tells it to not create an OpenGL context.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporarily disabled

    Window_ = glfwCreateWindow(WIDTH, HEIGHT, "IVR", nullptr, nullptr);
}

void IvrApp::initVulkan() {

    IVRInstanceCreator instance_creator(true);
    Instance_ = instance_creator.CreateVulkanInstance();

    createSurface();

    IVRDeviceCreator device_creator;
    PhysicalDevice_ = device_creator.PickPhysicalDevice(Surface_, Instance_);
    LogicalDevice_ = device_creator.CreateLogicalDevice(Surface_);
    GraphicsQueue_ = device_creator.GetGraphicsQueue();
    PresentQueue_ = device_creator.GetPresentQueue();

    uint32_t graphics_queue_family_index = FindQueueFamilies(PhysicalDevice_, Surface_).graphicsFamily;

    SwapchainManager_ = std::make_shared<IVRSwapchainManager>();
    SwapchainManager_->CreateSwapchain(LogicalDevice_, PhysicalDevice_, Surface_, device_creator.GetDeviceQueueFamilies());
    SwapchainManager_->CreateImageViews(LogicalDevice_);
     
    UniformBufferManager_ = std::make_shared<IVRUBManager>(LogicalDevice_, PhysicalDevice_, SwapchainManager_->GetImageViewCount());
    UniformBufferManager_->CreateUniformBuffers();

    std::shared_ptr<IVRTexObj> sample_texture = std::make_shared<IVRTexObj>(LogicalDevice_, PhysicalDevice_, 
                                                                        graphics_queue_family_index, GraphicsQueue_, "../texture_files/statue.jpg");
    std::shared_ptr<IVRDepthImage> depth_image = std::make_shared<IVRDepthImage>(LogicalDevice_, PhysicalDevice_, 
                                                                        graphics_queue_family_index, GraphicsQueue_, 
                                                                        SwapchainManager_->GetSwapchainExtent());

    PipelineManager_ = std::make_shared<IVRPipelineManager>(LogicalDevice_, SwapchainManager_, UniformBufferManager_, sample_texture, depth_image);
    GraphicsPipeline_ = PipelineManager_->CreateGraphicsPipeline();
    RenderPass_ = PipelineManager_->GetRenderPass();
    SwapchainManager_->CreateFramebuffers(RenderPass_, LogicalDevice_, depth_image->GetDepthImageView());
    
    Model_ = std::make_shared<IVRModel>(LogicalDevice_, PhysicalDevice_, GraphicsQueue_, graphics_queue_family_index);
    Model_->CreateVertexBuffer();
    Model_->CreateIndexBuffer();
    
    CommandBufferManager_ = std::make_shared<IVRCBManager>(PipelineManager_);
    CommandBufferManager_->CreateCommandPool(PhysicalDevice_, Surface_, LogicalDevice_);
    CommandBuffer_ = CommandBufferManager_->CreateCommandBuffer(LogicalDevice_);

    CreateSyncObjects();
}

void IvrApp::mainLoop() {
    while(!glfwWindowShouldClose(Window_))
    {
        glfwPollEvents();
        DrawFrame();
    }
}

void IvrApp::cleanup() {

    vkDestroySemaphore(LogicalDevice_, ImageAvailableSemaphore_, nullptr);
    vkDestroySemaphore(LogicalDevice_, RenderFinishedSemaphore_, nullptr);
    vkDestroyFence(LogicalDevice_, InFlightFence_, nullptr);

    SwapchainManager_->DestroySwapchain(LogicalDevice_);
    vkDestroySurfaceKHR(Instance_, Surface_, nullptr); //must be destroyed before the instance since it needs the instance
    vkDestroyInstance(Instance_, nullptr);
    glfwDestroyWindow(Window_);
    glfwTerminate();
}



void IvrApp::createSurface()
{
    //there are platform specific Vulkan functions to create the window surface using specific structs (like struct VkWin32SurfaceCreateInfoKHR and function vkCreateWin32SurfaceKHR)
    //but glfw gives us a platform agnostic way of creating the vulkan surface 

    if(glfwCreateWindowSurface(Instance_, Window_, nullptr, &Surface_) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface");
    }
}

void IvrApp::DrawFrame()
{
    //at the start of the frame, we want to wait for the previous frame to finish (so that command buffer and semaphores are available for use)
    vkWaitForFences(LogicalDevice_, 1, &InFlightFence_, VK_TRUE, UINT64_MAX);
    //in this case there is only one fence, so wait for all does not matter. Setting timeout to max disables the timeout

    //After waiting, the fence needs to be manually reset to the unsignalled state
    vkResetFences(LogicalDevice_, 1, &InFlightFence_);

    uint32_t image_index;
    //to draw a frame, we first need to get an image from the swapchain
    //the returned image_index refers to the VkImage in our swapchainImages array. This index is used to pick the framebuffer
    vkAcquireNextImageKHR(LogicalDevice_, SwapchainManager_->GetSwapchain(), UINT64_MAX, ImageAvailableSemaphore_, VK_NULL_HANDLE, &image_index);

    UpdateMVPUniformBuffer(image_index);

    vkResetCommandBuffer(CommandBuffer_, 0);
    CommandBufferManager_->RecordCommandBuffer(CommandBuffer_, image_index, RenderPass_, SwapchainManager_, GraphicsPipeline_, Model_);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {ImageAvailableSemaphore_};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = waitSemaphores;
    submit_info.pWaitDstStageMask = waitStages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &CommandBuffer_;

    VkSemaphore signal_semaphores[] = {RenderFinishedSemaphore_};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if(vkQueueSubmit(GraphicsQueue_, 1, &submit_info, InFlightFence_) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command");
    }

    //last step is to submit the result back to the swapchain to have it show up on the screen
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {SwapchainManager_->GetSwapchain()};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;
    
    vkQueuePresentKHR(PresentQueue_, &present_info);

}

void IvrApp::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; //creating the fence as signalled so that the first time it is used, it is in the signalled state (does not block)

    if(vkCreateSemaphore(LogicalDevice_, &semaphore_info, nullptr, &ImageAvailableSemaphore_) != VK_SUCCESS ||
        vkCreateSemaphore(LogicalDevice_, &semaphore_info, nullptr, &RenderFinishedSemaphore_) != VK_SUCCESS )
    {
        throw std::runtime_error("Failed to create semaphore");
    }

    if(vkCreateFence(LogicalDevice_, &fence_info, nullptr, &InFlightFence_) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create fence");
    }
}

void IvrApp::UpdateMVPUniformBuffer(uint32_t current_image_index)
{
    static auto start_time = std::chrono::high_resolution_clock::now();

    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

    MVPUniformBufferObject mvp_ubo{};
    mvp_ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    mvp_ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    mvp_ubo.proj = glm::perspective(glm::radians(45.0f), 
                    SwapchainManager_->GetSwapchainExtent().width / (float) SwapchainManager_->GetSwapchainExtent().height, 
                    0.1f, 10.0f);
    mvp_ubo.proj[1][1] *= -1; //glm was originally designed for OpenGL where the Y-coordinate of the clip coordinates is inverted
    //to compensate for this, the scaling factor of the Y-axis is sign-flipped

    memcpy(UniformBufferManager_->UniformBuffersMapped[current_image_index], &mvp_ubo, sizeof(MVPUniformBufferObject));
}

int main() {
    IvrApp app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}