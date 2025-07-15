#pragma once
#include <vector>
#include <memory>
#include <unordered_set>
#include "BlockMesh.h"
#include "Player.h"
#include "VectorUtils.h"
#include <thread>
#include "reputeless/PerlinNoise.hpp"
#include "Blocks.h"
#include <unordered_map>

class BlockMesh;

class World {
    public:
        World(int seed = 0);
        ~World();
        void update(double dt);
        void load(Texture& tex);
        bool isLoaded();
        void drawChunks();
        void updatePlayer(double dt);
        Player getPlayer();
        Vector3 getSpawn();
        RayCollision rayCollision(const Ray &ray, float distance);
        Block getBlockGlobal(Vector3 globalCoord);
        void setBlockGlobal(Vector3 globalCoord, Block block, bool updateMesh = false);
        void updateBlockGlobal(Vector3 globalCoord);
        void regenerateChunk(Vector2 chunkLoc);

    private:
        const siv::PerlinNoise::seed_type m_seed;
        const siv::PerlinNoise m_perlinNoise;
        float m_renderDistance = 10.5;
        std::unordered_map<Vector2, std::unique_ptr<BlockMesh>, Utils::Vector2Hash, Utils::Vector2Equal> m_chunks;
        Player m_player;
        bool m_running = true;
        std::thread m_chunkLoader;
        std::mutex m_chunkLock;


        static std::unordered_set<Vector2, Utils::Vector2Hash, Utils::Vector2Equal> genCirclePoints(float radius);
        void runChunkLoader();
};