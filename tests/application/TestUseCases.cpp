#include <iostream>
#include <cassert>
#include "application/Session.hpp"
#include "application/use_cases/CreateWorldUseCase.hpp"
#include "application/use_cases/OpenDatasetUseCase.hpp"
#include "application/ports/IFileSystem.hpp"

// Mock IFileSystem
class MockFileSystem : public Application::Ports::IFileSystem {
public:
    [[nodiscard]] bool exists(const std::string& path) const override {
        return path == "/exists.dat";
    }
    
    [[nodiscard]] std::string getAbsolutePath(const std::string& path) const override {
        return "/abs/" + path;
    }
};

void test_create_world() {
    Application::Session session;
    Application::UseCases::CreateWorldUseCase useCase(session);
    
    useCase.execute({"Mars", 500, 500});
    
    const auto& world = session.getWorkspace().getWorld();
    assert(world != nullptr);
    assert(world->getName() == "Mars");
    std::cout << "CreateWorldUseCase passed!" << std::endl;
}

void test_open_dataset() {
    Application::Session session;
    MockFileSystem fs;
    Application::UseCases::OpenDatasetUseCase useCase(session, fs);
    
    // Test success
    useCase.execute({"Data1", "/exists.dat"});
    const auto& datasets = session.getWorkspace().getDatasets();
    assert(datasets.size() == 1);
    assert(datasets[0]->getName() == "Data1");

    // Test failure
    bool caught = false;
    try {
        useCase.execute({"Data2", "/missing.dat"});
    } catch (const std::runtime_error&) {
        caught = true;
    }
    assert(caught);

    std::cout << "OpenDatasetUseCase passed!" << std::endl;
}

int main() {
    test_create_world();
    test_open_dataset();
    return 0;
}
