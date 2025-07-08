#pragma once
#include <vector>
#include <memory>
#include <unordered_set>
#include "BlockMesh.h"
#include "MyCamera.h"
#include "VectorUtils.h"

class World {
    public:
        World(int seed = 0);
        void update(double dt);
        void drawChunks();
        void updatePlayer(double dt);
        MyCamera getPlayer();
        Vector3 getSpawn();
    private:
        const int m_seed;
        int m_renderDistance = 4;
        std::vector<std::unique_ptr<BlockMesh>> m_chunks;
        MyCamera m_player;

        static std::unordered_set<Vector2, Utils::Vector2Hash, Utils::Vector2Equal> genCirclePoints(float radius);
};