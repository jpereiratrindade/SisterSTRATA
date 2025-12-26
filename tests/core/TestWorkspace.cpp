#include <iostream>
#include <cassert>
#include "core/domain/Workspace.hpp"

void test_workspace_creation() {
    Core::Domain::Workspace workspace;
    
    // Test dataset addition
    workspace.addDataset("TestSet", "/tmp/test.dat");
    const auto& datasets = workspace.getDatasets();
    assert(datasets.size() == 1);
    assert(datasets[0]->getName() == "TestSet");
    assert(datasets[0]->getPath().toString() == "/tmp/test.dat");

    // Test world creation
    workspace.createWorld("MyWorld", 1024, 768);
    const auto& world = workspace.getWorld();
    assert(world != nullptr);
    assert(world->getName() == "MyWorld");
    assert(world->getResolution().width == 1024);
    assert(world->getResolution().height == 768);

    std::cout << "Workspace tests passed!" << std::endl;
}

int main() {
    test_workspace_creation();
    return 0;
}
