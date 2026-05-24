#include "TreeNode.h"
#include "NodeUtils.h"

std::shared_ptr<SceneNode> buildTreeGroup()
{
    auto group = std::make_shared<SceneNode>();

    // ── Round cone-top trees ──────────────────────────────
    struct TreePos { float x, z; };
    static const TreePos roundTrees[] = {
        {-19.f, -25.f}, { 19.f, -20.f},
        {-19.f,  20.f}, { 19.f,  20.f},
        {-10.f,  26.f}, { 10.f,  26.f},
    };
    for(auto& t : roundTrees){
        auto treeNode = std::make_shared<SceneNode>();
        auto* trunk = makeShape<Cylinder>(t.x, 0.4f, t.z, 0.15f, 0.8f, 8,
                                          0.45f, 0.28f, 0.12f);
        auto* top   = makeShape<Cone>    (t.x, 1.2f, t.z, 0.55f, 1.1f, 8,
                                          0.18f, 0.52f, 0.18f);
        trunk->build(); top->build();
        treeNode->addChild(shapeNode(trunk));
        treeNode->addChild(shapeNode(top));
        group->addChild(treeNode);
    }

    // ── Prism-top trees ───────────────────────────────────
    static const TreePos prismTrees[] = {
        {-19.f,  5.f}, { 19.f, -5.f},
        {-15.f, 26.f}, { 15.f,  26.f},
    };
    for(auto& t : prismTrees){
        auto treeNode = std::make_shared<SceneNode>();
        auto* trunk = makeShape<Cylinder>(t.x, 0.4f, t.z, 0.15f, 0.8f, 8,
                                          0.30f, 0.18f, 0.08f);
        auto* top   = makeShape<TriangularPrism>(t.x, 1.2f, t.z, 0.55f, 0.55f, 0.55f,
                                                  0.12f, 0.38f, 0.12f);
        trunk->build(); top->build();
        treeNode->addChild(shapeNode(trunk));
        treeNode->addChild(shapeNode(top));
        group->addChild(treeNode);
    }

    return group;
}
