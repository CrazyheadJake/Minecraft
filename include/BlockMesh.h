#pragma once
#include <cstdint>
#include "raylib.h"
#include <array>
#include "Blocks.h"
#include "MyCamera.h"

class BlockMesh {
    public:
        static constexpr int LENGTH = 16;
        static constexpr int HEIGHT = 32;
        BlockMesh(Vector3 offset = {0, 0, 0});
        BlockMesh(const BlockMesh&) = delete;            // No copy constructor
        BlockMesh& operator=(const BlockMesh&) = delete; // No copy assignment
        ~BlockMesh();
        void drawMesh() const;
        void generateMeshes();
        void generateWorld();
        bool isValid();
        bool isVisible(MyCamera& camera) const;
        void tryUploadMeshes();
        Vector3 getLocalCoord(int i) const;
        Vector3 getGlobalCoord(int i) const;
        Vector2 getChunkLoc() const;
        void setBlock(int x, int y, int z, Block block);
        Block getBlockLocal(Vector3 localCoord) const;

    private:
        enum class State {
            UNINITIALIZED,
            MESH_GENERATED,
            MESH_UPLOADED
        };
        static const unsigned short MAX_VERTS = UINT16_MAX;
        State m_state = State::UNINITIALIZED;
        std::array<Block, LENGTH*LENGTH*HEIGHT> m_blocks;
        std::vector<Mesh> m_meshes;
        Model m_model = {0};
        BoundingBox m_boundingBox = {0};

        Vector3 m_chunkOffset;

        void addMesh(const std::vector<Vector3>& vertices, const std::vector<unsigned short>& indices, const std::vector<Vector2>& texcoords, const std::vector<Vector3>& normals);
        void generateModel();
        void uploadMeshes();
        Vector3 getCorner(int i) const;

};