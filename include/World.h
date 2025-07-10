#pragma once
#include <vector>
#include <memory>
#include <unordered_set>
#include "BlockMesh.h"
#include "Player.h"
#include "VectorUtils.h"
#include <thread>
#include "reputeless/PerlinNoise.hpp"

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
    private:
        const siv::PerlinNoise::seed_type m_seed;
        const siv::PerlinNoise m_perlinNoise;
        float m_renderDistance = 10.5;
        std::vector<std::unique_ptr<BlockMesh>> m_chunks;
        Player m_player;
        bool m_running = true;
        std::thread m_chunkLoader;
        std::mutex m_chunkLock;


        static std::unordered_set<Vector2, Utils::Vector2Hash, Utils::Vector2Equal> genCirclePoints(float radius);
        void runChunkLoader();
};