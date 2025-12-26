#include "world3d/Engine.hpp"
#include "world3d/ScientificAdapter.hpp"
#include "core/value_objects/Vector3.hpp"
#include <SDL2/SDL_vulkan.h>
#include <iostream>

namespace World3D {

Engine::Engine() {}

Engine::~Engine() {
    shutdown();
}

void Engine::init(SDL_Window* window) {
    std::cout << "[Engine] Initializing..." << std::endl;
    
    context_ = std::make_shared<Rendering::VulkanContext>(window, "SisterPEC");
    
    int windowW = 0, windowH = 0;
    SDL_GetWindowSize(window, &windowW, &windowH);

    int drawableW = 0, drawableH = 0;
    SDL_Vulkan_GetDrawableSize(window, &drawableW, &drawableH);
    if (drawableW <= 0 || drawableH <= 0) {
        drawableW = windowW;
        drawableH = windowH;
    }

    renderer_ = std::make_unique<Rendering::VulkanRenderer>(context_, drawableW, drawableH);
    
    // Camera positioned to see the grid from a slight overhead angle
    camera_ = std::make_unique<Camera>(glm::vec3(0.0f, -150.0f, 50.0f), 45.0f, (float)windowW / (float)windowH);
    
    // Constructor faces +Y; tilt down a bit to look at the origin.
    camera_->rotate(0.0f, -15.0f); 

    inputController_ = std::make_unique<CameraInputController>(*camera_);

    // Init ThreadPool
    threadPool_ = std::make_unique<Infrastructure::Threading::ThreadPool>(4);

    // Descriptor Pool
    vk::DescriptorPoolSize pool_sizes[] = {
        { vk::DescriptorType::eUniformBuffer, 1000 },
        { vk::DescriptorType::eCombinedImageSampler, 1000 }
    };
    vk::DescriptorPoolCreateInfo pool_info = {};
    pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    descriptorPool_ = context_->getDevice().createDescriptorPool(pool_info);
    
    // Create reference grid
    uploadDemoData();

    std::cout << "[Engine] Initialized." << std::endl;
}

void Engine::shutdown() {
    if (descriptorPool_) {
         if (context_) {
            context_->getDevice().waitIdle();
            context_->getDevice().destroyDescriptorPool(descriptorPool_);
         }
        descriptorPool_ = nullptr;
    }
    
    // Clear scene buffers before context is destroyed
    scene_.clear();
    
    renderer_.reset();
    inputController_.reset();
    camera_.reset();
    context_.reset();
}

void Engine::processEvent(const SDL_Event& event) {
    if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            framebufferResized_ = true;
        }
    }

    if (inputController_) {
        inputController_->processEvent(event);
    }
}

void Engine::update(float deltaTime) {
    // Process Async Tasks on Main Thread
    commandQueue_.processAll();

    if (inputController_) {
        inputController_->update(deltaTime);
    }
}

void Engine::render(std::function<void(vk::CommandBuffer)> overlayRender) {
    if (renderer_ && camera_) {
        if (framebufferResized_) {
            int w = 0, h = 0;
            SDL_Window* window = context_->getWindow();
            SDL_GetWindowSize(window, &w, &h);
            if (w > 0 && h > 0) {
                 renderer_->recreateSwapchain();
                 // Update Camera Aspect Ratio
                 camera_->setAspectRatio((float)w / (float)h);
                 framebufferResized_ = false;
            }
        }
    
        renderer_->beginFrame(*camera_);
        renderer_->render(scene_);
        
        if (overlayRender) {
            // We need to pass the CURRENT command buffer.
            // Assuming renderer exposes it or accessible via Engine::getRenderer
            auto cmd = renderer_->getCommandBuffers()[renderer_->getCurrentFrameIndex()];
            overlayRender(cmd);
        }

        renderer_->endFrame();
    }
}

void Engine::uploadDemoData() {
    if (!renderer_) return;

    // --- 1. Grid Generation (Now separate from Renderer) ---
    std::vector<Rendering::Vertex> gridVertices;
    int size = 100; // 100 lines each way
    float spacing = 2.0f; // 2 meters spacing
    float halfSize = (size * spacing) / 2.0f;
    glm::vec3 gridColor(0.5f, 0.5f, 0.5f);
    glm::vec3 xAxisColor(1.0f, 0.0f, 0.0f);
    glm::vec3 yAxisColor(0.0f, 1.0f, 0.0f);

    for (int i = 0; i <= size; i++) {
        float pos = -halfSize + (i * spacing);
        gridVertices.push_back({{pos, -halfSize, 0.0f}, gridColor, {0,0,1}, {0,0}});
        gridVertices.push_back({{pos, halfSize, 0.0f}, gridColor, {0,0,1}, {0,0}});
        gridVertices.push_back({{-halfSize, pos, 0.0f}, gridColor, {0,0,1}, {0,0}});
        gridVertices.push_back({{halfSize, pos, 0.0f}, gridColor, {0,0,1}, {0,0}});
    }
    gridVertices.push_back({{-halfSize, 0.0f, 0.1f}, xAxisColor, {0,0,1}, {0,0}});
    gridVertices.push_back({{halfSize, 0.0f, 0.1f}, xAxisColor, {0,0,1}, {0,0}});
    gridVertices.push_back({{0.0f, -halfSize, 0.1f}, yAxisColor, {0,0,1}, {0,0}});
    gridVertices.push_back({{0.0f, halfSize, 0.1f}, yAxisColor, {0,0,1}, {0,0}});

    // Create Grid RenderObject
    RenderObject gridObj;
    gridObj.topology = vk::PrimitiveTopology::eLineList;
    gridObj.vertexCount = static_cast<uint32_t>(gridVertices.size());
    
    // Create Buffer
    vk::DeviceSize gridSize = sizeof(Rendering::Vertex) * gridVertices.size();
    Rendering::Buffer gridStaging(*context_, gridSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    gridStaging.copyTo(gridVertices.data(), gridSize);
    
    gridObj.vertexBuffer = std::make_shared<Rendering::Buffer>(*context_, gridSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
    
    renderer_->copyBuffer(gridStaging.getHandle(), gridObj.vertexBuffer->getHandle(), gridSize);
    
    // --- 2. Points Generation ---
    Core::ValueObjects::Vector3 origin(500000.0, 7000000.0, 0.0);
    std::vector<Core::ValueObjects::Vector3> rawPoints;
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            double x = origin.x + (i - 25) * 0.5;
            double y = origin.y + (j - 25) * 0.5;
            double z = std::sin(i * 0.2) * std::cos(j * 0.2) * 2.0;
            rawPoints.push_back({x, y, z});
        }
    }
    auto gpuVertices = ScientificAdapter::convert(rawPoints, origin);
    
    RenderObject pointsObj;
    pointsObj.topology = vk::PrimitiveTopology::ePointList;
    pointsObj.vertexCount = static_cast<uint32_t>(gpuVertices.size());
    vk::DeviceSize pointsSize = sizeof(Rendering::Vertex) * gpuVertices.size();
    
    Rendering::Buffer pointsStaging(*context_, pointsSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    pointsStaging.copyTo(gpuVertices.data(), pointsSize);
    
    pointsObj.vertexBuffer = std::make_shared<Rendering::Buffer>(*context_, pointsSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    renderer_->copyBuffer(pointsStaging.getHandle(), pointsObj.vertexBuffer->getHandle(), pointsSize);
    
    scene_.addObject(gridObj);
    scene_.addObject(pointsObj);
    
    std::cout << "[Engine] Uploaded demo data to Scene." << std::endl;
}

void Engine::uploadDemoDataAsync() {
    std::cout << "[Engine] Starting Async Data Generation..." << std::endl;
    
    // 1. Submit Heavy Task to Background Thread
    threadPool_->submit([this]() {
        // --- Heavy Calculation (Simulated) ---
        // Let's generate MORE points to make it noticeable? Or just sleep?
        // Let's generate a massive distinct point cloud.
        
        Core::ValueObjects::Vector3 origin(500000.0, 7000000.0, 0.0);
        std::vector<Core::ValueObjects::Vector3> rawPoints;
        int dim = 100; // 100x100 = 10k points
        
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                // Heavier math
                double x = origin.x + (i - dim/2) * 1.0;
                double y = origin.y + (j - dim/2) * 1.0;
                double z = std::sin(i * 0.1) * std::cos(j * 0.1) * 10.0;
                // Artificial delay to prove async
                // std::this_thread::sleep_for(std::chrono::microseconds(1)); 
                rawPoints.push_back({x, y, z});
            }
        }
         // Simulate IO delay
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Convert to GPU format (still safe in background as it's just data manipulation)
        auto gpuVertices = ScientificAdapter::convert(rawPoints, origin);
        
        std::cout << "[Async] Generation Complete. Points: " << gpuVertices.size() << std::endl;

        // 2. Enqueue Upload Task to Main Thread
        // We need to capture the data by value (move) or shared_ptr to keep it alive
        auto verticesPtr = std::make_shared<std::vector<Rendering::Vertex>>(std::move(gpuVertices));

        commandQueue_.push([this, verticesPtr]() {
            std::cout << "[MainThread] Uploading Async Data to GPU..." << std::endl;
            
            if (!context_ || !renderer_) return;

            RenderObject pointsObj;
            pointsObj.topology = vk::PrimitiveTopology::ePointList;
            pointsObj.vertexCount = static_cast<uint32_t>(verticesPtr->size());
            vk::DeviceSize pointsSize = sizeof(Rendering::Vertex) * verticesPtr->size();
            
            Rendering::Buffer pointsStaging(*context_, pointsSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            pointsStaging.copyTo(verticesPtr->data(), pointsSize);
            
            pointsObj.vertexBuffer = std::make_shared<Rendering::Buffer>(*context_, pointsSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

            renderer_->copyBuffer(pointsStaging.getHandle(), pointsObj.vertexBuffer->getHandle(), pointsSize);
            
            scene_.addObject(pointsObj);
            std::cout << "[MainThread] Async Data Upload Complete!" << std::endl;
        });
    });
}

} // namespace World3D
