#pragma once
#include <cstdint>
#include "raylib.h"
#include <array>
#include "Block.h"
#include "Player.h"
#include "reputeless/PerlinNoise.hpp"
#include "World.h"
#include <unordered_map>
#include "VectorUtils.h"

class World;

class BlockMesh {
    public:
        static constexpr int LENGTH = 16;
        static constexpr int HEIGHT = 256;

        BlockMesh(World& world, const siv::PerlinNoise& perlin, Vector3 offset);
        BlockMesh(const BlockMesh&) = delete;            // No copy constructor
        BlockMesh& operator=(const BlockMesh&) = delete; // No copy assignment
        ~BlockMesh();
        void drawMesh() const;
        void drawTransparentMesh() const;
        bool isValid() const;
        bool isVisible(Player& camera) const;
        void tryUploadMeshes();
        Vector3 getLocalCoord(int i) const;
        Vector3 getGlobalCoord(int i) const;
        Vector2 getChunkLoc() const;
        void setBlock(Vector3 localCoord, Block block, bool updateMesh = false);
        void setBlock(int x, int y, int z, Block block, bool updateMesh = false);
        Block getBlockLocal(Vector3 localCoord) const;
        bool isLocalCoord(Vector3 localCoord) const;
        void tryGenerateMeshData();
        void tryGenerateMeshes();
        void updateBlockData(Vector3 localCoord);
        void genBlockData(Vector3 localCoord);
        void requestRegenerate();
        void generateMeshData();
        void generateMeshesFromData();
        void updateTransparentMesh(Vector3 location);
        void updateTransparentMesh(Vector3 location, Mesh* meshes, int meshCount, int transparentMeshCount);
        bool shouldRegenerate() const;
        void lock();
        void unlock();

    private:
        struct BlockData {
            std::vector<Vector3> vertices;
            std::vector<unsigned short> indices;
            std::vector<Vector2> texcoords;
            std::vector<Vector3> normals;
            bool transparent;
            Block block;
        };
        enum class State {
            UNINITIALIZED,
            WORLD_GENERATED,
            MESHDATA_GENERATED,
            MESH_GENERATED,
            MESH_UPLOADED
        };
        static constexpr int SEALEVEL = 64; // Sea level for the world generation
        static constexpr double SCALE = 0.02f; // Scale for the Perlin noise
        static const unsigned short MAX_VERTS = UINT16_MAX;
        
        bool m_regenerate = false;
        State m_state = State::UNINITIALIZED;

        std::array<Block, LENGTH*LENGTH*HEIGHT> m_blocks;
        std::unordered_map<int, BlockData> m_meshData;
        int m_verticesCount = 0;
        std::vector<const BlockData*> m_transparentBlocks;
        int m_transparentVerticesCount = 0;
        int m_transparentMeshCount = 0;
        Model m_model = {0};
        BoundingBox m_boundingBox = {0};

        Vector3 m_chunkOffset;
        World& m_world;
        std::mutex m_dataLock;

        void clearMeshData();        
        void clearMeshes();
        void clearModel();
        void generateModel();
        void generateWorld(const siv::PerlinNoise& perlin);
        void uploadMeshes();
        Vector3 getCorner(int i) const;

};