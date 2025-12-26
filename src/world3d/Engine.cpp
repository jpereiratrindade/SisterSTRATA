#include "world3d/Engine.hpp"
#include "world3d/ScientificAdapter.hpp"
#include "core/value_objects/Vector3.hpp"
#include <iostream>

namespace World3D {

Engine::Engine() {}

Engine::~Engine() {
    shutdown();
}

void Engine::init(SDL_Window* window) {
    std::cout << "[Engine] Initializing..." << std::endl;
    
    context_ = std::make_shared<Rendering::VulkanContext>(window, "SisterPEC");
    
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    renderer_ = std::make_unique<Rendering::VulkanRenderer>(context_, w, h);
    
    // Camera
    camera_ = std::make_unique<Camera>(glm::vec3(0.0f, -50.0f, 20.0f), 45.0f, (float)w / (float)h);
    inputController_ = std::make_unique<CameraInputController>(*camera_);

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
    if (inputController_) {
        inputController_->processEvent(event);
    }
}

void Engine::update(float deltaTime) {
    if (inputController_) {
        inputController_->update(deltaTime);
    }
}

void Engine::render() {
    if (renderer_ && camera_) {
        renderer_->beginFrame(*camera_);
        renderer_->render(scene_);
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
        gridVertices.push_back({{pos, -halfSize, 0.0f}, gridColor});
        gridVertices.push_back({{pos, halfSize, 0.0f}, gridColor});
        gridVertices.push_back({{-halfSize, pos, 0.0f}, gridColor});
        gridVertices.push_back({{halfSize, pos, 0.0f}, gridColor});
    }
    gridVertices.push_back({{-halfSize, 0.0f, 0.1f}, xAxisColor});
    gridVertices.push_back({{halfSize, 0.0f, 0.1f}, xAxisColor});
    gridVertices.push_back({{0.0f, -halfSize, 0.1f}, yAxisColor});
    gridVertices.push_back({{0.0f, halfSize, 0.1f}, yAxisColor});

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

} // namespace World3D
