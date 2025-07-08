#include "World.h"
#include "raymath.h"
#include <unordered_set>
#include "VectorUtils.h"
#include <iostream>

World::World(int seed): m_seed(seed)
{
    m_player = MyCamera{
        getSpawn(), // position
        Vector3{1, 0, 0}, // fwd
        Vector3{0, 1, 0}, // up
        Vector3{0, 0, 1}, // right
        90.0f, // fovy
        CAMERA_PERSPECTIVE // perspective
    };
}

void World::update(double dt)
{
    if (IsKeyPressed(KEY_F2)) {
        EnableCursor();
        std::cout << "DEBUGGING" << std::endl;
    }
    auto chunkLocs = World::genCirclePoints(m_renderDistance);
    Vector2 playerChunk = Utils::floorVector(Vector2{m_player.position.x, m_player.position.z}, 16) / 16;
    for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
        // If chunk is too far away, unload it
        Vector2 chunkLoc = (*it)->getChunkLoc() - playerChunk;
        if (chunkLocs.find(chunkLoc) == chunkLocs.end()) {
            m_chunks.erase(it);
        }
        else {
            size_t erased = chunkLocs.erase(chunkLoc);
            if (erased == 0) {
                std::cerr << "Error: Chunk location not found in set." << std::endl;
            }
            if (erased > 1) {
                std::cerr << "Warning: More than one chunk location removed from set." << std::endl;
            }
            it++;
        }
    }
    for (Vector2 chunkLoc: chunkLocs) {
        chunkLoc += playerChunk; // Offset chunk location to player's position
        m_chunks.emplace_back(std::make_unique<BlockMesh>(Vector3{chunkLoc.x * BlockMesh::LENGTH, 0, chunkLoc.y * BlockMesh::WIDTH}));
    }

    updatePlayer(dt);
}

void World::drawChunks()
{
    for (const auto& chunk: m_chunks) {
        chunk->drawMesh();
    }
}

void World::updatePlayer(double dt)
{
    Vector2 dXY = GetMouseDelta();
    m_player.changeYaw(dXY.x * m_player.rotateSpeed);
    m_player.changePitch(dXY.y * m_player.rotateSpeed);
    m_player.update();
    
    if (IsKeyDown(KEY_W)) {
        m_player.moveFwd(dt);
    }
    if (IsKeyDown(KEY_S)) {
        m_player.moveFwd(-dt);
    }
    if (IsKeyDown(KEY_D)) {
        // EnableCursor();
        m_player.moveRight(dt);
        // DisableCursor();
    }
    if (IsKeyDown(KEY_A)) {
        m_player.moveRight(-dt);
    }
    if (IsKeyDown(KEY_SPACE)) {
        m_player.position += m_player.up * dt * m_player.speed;
    }
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        m_player.position -= m_player.up * dt * m_player.speed;
    }
}

MyCamera World::getPlayer()
{
    return m_player;
}

Vector3 World::getSpawn()
{
    return {BlockMesh::LENGTH/2, BlockMesh::HEIGHT, BlockMesh::WIDTH/2};
}

std::unordered_set<Vector2, Utils::Vector2Hash, Utils::Vector2Equal> World::genCirclePoints(float radius)
{
    std::unordered_set<Vector2, Utils::Vector2Hash, Utils::Vector2Equal> set = {};
    for (int i = 0; i <= radius; i++) {
        for (int k = 0; k <= radius; k++) {
            if (i*i+k*k <= radius*radius) {
                set.insert(Vector2{(float)i,(float)k});
                set.insert(Vector2{(float)i,(float)-k});
                set.insert(Vector2{(float)-i,(float)k});
                set.insert(Vector2{(float)-i,(float)-k});
                continue;
            }
            break;
        }
    }
    return set;
}
