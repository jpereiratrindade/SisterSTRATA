#include <gtest/gtest.h>

#include "core/domain/Workspace.hpp"

TEST(CoreWorkspaceTest, CreatesWorkspaceDatasetAndWorld) {
    Core::Domain::Workspace workspace;

    workspace.addDataset("TestSet", "/tmp/test.dat");
    const auto& datasets = workspace.getDatasets();
    ASSERT_EQ(datasets.size(), 1u);
    EXPECT_EQ(datasets[0]->getName(), "TestSet");
    EXPECT_EQ(datasets[0]->getPath().toString(), "/tmp/test.dat");

    workspace.createWorld("MyWorld", 1024, 768);
    const auto& world = workspace.getWorld();
    ASSERT_NE(world, nullptr);
    EXPECT_EQ(world->getName(), "MyWorld");
    EXPECT_EQ(world->getResolution().width, 1024);
    EXPECT_EQ(world->getResolution().height, 768);
}
