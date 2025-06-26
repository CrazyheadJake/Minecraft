#pragma once
#include <cstdint>
#include "raylib.h"
#include <array>
#include "Blocks.h"

class BlockMesh {
    public:
        BlockMesh(Vector3 offset = {0, 0, 0});
        ~BlockMesh();
        void drawMesh();
        void generateMesh();
        void updateMesh();
        Vector3 getGlobalCoord(int i);

    private:
        static constexpr int LENGTH = 16;
        static constexpr int WIDTH = 16;
        static constexpr int HEIGHT = 16;
        
        std::array<Block, LENGTH*WIDTH*HEIGHT> m_blocks;
        Model m_model;
        Mesh m_mesh;
        std::vector<Vector3> m_vertices;
        std::vector<Vector2> m_texcoords;
        std::vector<unsigned short> m_indices;

        Image m_image;
        Texture m_texture;

        Vector3 m_chunkOffset;
};