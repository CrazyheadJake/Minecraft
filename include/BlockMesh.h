#pragma once
#include <cstdint>
#include "raylib.h"
#include <array>
#include "Blocks.h"

class BlockMesh {
    public:
        BlockMesh(Vector3 offset = {0, 0, 0});
        BlockMesh(const BlockMesh&) = delete;            // No copy constructor
        BlockMesh& operator=(const BlockMesh&) = delete; // No copy assignment
        ~BlockMesh();
        void drawMesh() const;
        void generateMeshes();
        void generateWorld();
        void updateMesh();
        Vector3 getGlobalCoord(int i);
        void setBlock(int x, int y, int z, Block block);

    private:
        static constexpr int LENGTH = 16;
        static constexpr int WIDTH = 16;
        static constexpr int HEIGHT = 32;
        static const unsigned short MAX_VERTS = UINT16_MAX;
        
        std::array<Block, LENGTH*WIDTH*HEIGHT> m_blocks;
        Model m_model = {0};
        std::vector<Mesh> m_meshes;

        Vector3 m_chunkOffset;

        void addMesh(const std::vector<Vector3>& vertices, const std::vector<unsigned short>& indices, const std::vector<Vector2>& texcoords, const std::vector<Vector3>& normals);
        void generateModel();
};