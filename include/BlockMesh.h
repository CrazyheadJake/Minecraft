#pragma once
#include <cstdint>
#include "raylib.h"
#include <array>
#include "Blocks.h"
#include "Player.h"
#include "reputeless/PerlinNoise.hpp"

class BlockMesh {
    public:
        static constexpr int LENGTH = 16;
        static constexpr int HEIGHT = 256;
        BlockMesh(const siv::PerlinNoise& perlin, Vector3 offset);
        BlockMesh(const BlockMesh&) = delete;            // No copy constructor
        BlockMesh& operator=(const BlockMesh&) = delete; // No copy assignment
        ~BlockMesh();
        void drawMesh() const;
        bool isValid();
        bool isVisible(Player& camera) const;
        void tryUploadMeshes();
        Vector3 getLocalCoord(int i) const;
        Vector3 getGlobalCoord(int i) const;
        Vector2 getChunkLoc() const;
        void setBlock(int x, int y, int z, Block block, bool updateMesh = false);
        Block getBlockLocal(Vector3 localCoord) const;
        RayCollision rayCollision(const Ray& ray) const;

    private:
        enum class State {
            UNINITIALIZED,
            MESH_GENERATED,
            MESH_UPLOADED
        };
        static constexpr int SEALEVEL = 64; // Sea level for the world generation
        static constexpr double SCALE = 0.02f; // Scale for the Perlin noise
        static const unsigned short MAX_VERTS = UINT16_MAX;
        State m_state = State::UNINITIALIZED;
        std::array<Block, LENGTH*LENGTH*HEIGHT> m_blocks;
        std::vector<Mesh> m_meshes;
        Model m_model = {0};
        BoundingBox m_boundingBox = {0};

        Vector3 m_chunkOffset;
        

        void generateMeshes();
        void clearMeshes();
        void addMesh(const std::vector<Vector3>& vertices, const std::vector<unsigned short>& indices, const std::vector<Vector2>& texcoords, const std::vector<Vector3>& normals);
        void generateModel();
        void generateWorld(const siv::PerlinNoise& perlin);
        void uploadMeshes();
        Vector3 getCorner(int i) const;

};