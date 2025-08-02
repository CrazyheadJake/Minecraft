#include "Structure.h"
#include "Block.h"

void Structure::addStructure(const std::vector<std::vector<Block>>& structure, Vector3 dims)
{
    std::vector<BlockInstance> structureBlocks;
    for (int x = 0; x < dims.x; x++) {
        for (int y = 0; y < dims.y; y++) {
            for (int z = 0; z < dims.z; z++) {
                if (structure[y][x+z*dims.x] != Blocks::UNKNOWN) {
                    structureBlocks.push_back(BlockInstance {structure[y][x+z*dims.x], {(float)x - (int)dims.x/2, (float)y, (float)z - (int)dims.z/2}});
                }
            }
        }
    }
    m_structures.push_back(structureBlocks);
}

void Structures::generateStructs()
{
    std::vector<std::vector<Block>> tree;
    tree.push_back({
        {}, {}, {}, {}, {},
        {}, {}, {}, {}, {},
        {}, {}, {Blocks::OAK_LOG}, {}, {},
        {}, {}, {}, {}, {},
        {}, {}, {}, {}, {},
    });
    tree.push_back({
        {}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {},
        {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES},
        {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LOG}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES},
        {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES},
        {}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {},
    });
    tree.push_back({
        {}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {},
        {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES},
        {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LOG}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES},
        {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES},
        {}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {},
    });
    tree.push_back({
        {}, {}, {}, {}, {},
        {}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {},
        {}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LOG}, {Blocks::OAK_LEAVES}, {},
        {}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {},
        {}, {}, {}, {}, {},
    });
    tree.push_back({
        {}, {}, {}, {}, {},
        {}, {}, {Blocks::OAK_LEAVES}, {}, {},
        {}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {Blocks::OAK_LEAVES}, {},
        {}, {}, {Blocks::OAK_LEAVES}, {}, {},
        {}, {}, {}, {}, {},
    });

    TREE.addStructure(tree, {5, 5, 5});

    tree.insert(tree.begin(), std::vector<Block>
                           {{}, {}, {}, {}, {},
                            {}, {}, {}, {}, {},
                            {}, {}, {Blocks::OAK_LOG}, {}, {},
                            {}, {}, {}, {}, {},
                            {}, {}, {}, {}, {}});


    TREE.addStructure(tree, {5, 6, 5});

    tree.insert(tree.begin(), std::vector<Block>
                           {{}, {}, {}, {}, {},
                            {}, {}, {}, {}, {},
                            {}, {}, {Blocks::OAK_LOG}, {}, {},
                            {}, {}, {}, {}, {},
                            {}, {}, {}, {}, {}});

    TREE.addStructure(tree, {5, 7, 5});

}
