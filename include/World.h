#pragma once
#include <vector>
#include <memory>
#include <unordered_set>
#include "BlockMesh.h"
#include "Player.h"
#include "VectorUtils.h"
#include <thread>
#include "reputeless/PerlinNoise.hpp"
#include "Block.h"
#include <unordered_map>
#include <atomic>

class BlockMesh;

class World {
public:
    World(int seed = 0);
    ~World();
    void update(double dt);
    void load(Texture& tex);
    void drawChunks();
    void drawSkybox();
    void updatePlayer(double dt); 
    const Player& getPlayer() const;
    Vector3 getSpawn() const;
    RayCollision rayCollision(const Ray &ray, float distance) const;
    BlockData getBlockGlobal(Vector3 globalCoord) const;
    void setBlockGlobal(Vector3 globalCoord, Block block, bool updateMesh = false);
    void updateBlockGlobal(Vector3 globalCoord);
    void updateLightGlobal(Vector3 globalCoord);
    void regenerateChunk(Vector2 chunkLoc);
    void relightChunk(Vector2 chunkLoc);
    std::vector<BlockInstance> getFutureBlocks(Vector2 chunkLoc);
    

private:
    static const int TIMELOC = 0;
    const siv::PerlinNoise::seed_type m_seed;
    const siv::PerlinNoise m_perlinNoise;

    std::unordered_map<Vector2, std::unique_ptr<BlockMesh>, Utils::Vector2Hash, Utils::Vector2Equal> m_chunks;
    std::unordered_map<Vector2, std::vector<BlockInstance>, Utils::Vector2Hash, Utils::Vector2Equal> m_futureBlocks;
    std::vector<std::reference_wrapper<BlockMesh>> m_sortedChunks;
    mutable std::mutex m_chunkLock;
    std::atomic<bool> m_running = true;
    std::thread m_chunkLoader;

    Player m_player;
    float m_renderDistance = 1;
    Model m_skybox;
    Model m_sun;
    Model m_moon;
    float m_time;

    static std::unordered_set<Vector2, Utils::Vector2Hash, Utils::Vector2Equal> genCirclePoints(float radius);
    static std::unordered_set<Vector2, Utils::Vector2Hash, Utils::Vector2Equal> genSquarePoints(float radius);
    void runChunkLoader();
    void sortChunks(Vector2 playerChunkLoc);
    inline float getDayTime();
};