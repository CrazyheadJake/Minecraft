#pragma once
#include <vector>
#include "Block.h"

class Structure
{
public:
    Structure() {};
    // Delete copy and move semantics
    Structure(const Structure&) = delete;
    Structure& operator=(const Structure&) = delete;
    Structure(Structure&&) = delete;
    Structure& operator=(Structure&&) = delete;

    void addStructure(const std::vector<std::vector<Block>>& structure, Vector3 dims);
    const std::vector<BlockInstance>& getStructure(int i) { return m_structures[i]; };
    int getStructureCount() const { return m_structures.size(); }

    static std::vector<BlockInstance> rotateStructure(const std::vector<BlockInstance>& structure);

private:
    std::vector<std::vector<BlockInstance>> m_structures;
};

namespace Structures {
    inline Structure TREE;

    extern void generateStructs();
};
