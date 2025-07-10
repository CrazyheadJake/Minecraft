#include "World.h"
#include "raymath.h"
#include <unordered_set>
#include "VectorUtils.h"
#include <iostream>
#include <thread>

World::World(int seed): m_seed(seed), m_perlinNoise(m_seed), m_chunkLoader(&World::runChunkLoader, this), m_player(getSpawn(), 90.0f, CAMERA_PERSPECTIVE)
{
}

World::~World() {
    m_running = false;
    if (m_chunkLoader.joinable())
        m_chunkLoader.join();
}

void World::update(double dt)
{
    if (IsKeyPressed(KEY_F2)) {
        EnableCursor();
        std::cout << "DEBUGGING" << std::endl;
    }
    
    updatePlayer(dt);
}

void World::load(Texture& tex)
{   
    bool loading = true;
    while (loading && !WindowShouldClose()) {
        // Draw loading screen
        BeginDrawing();
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), LIGHTGRAY);
        DrawTexture(tex, 
                    GetScreenWidth()/2 - tex.width/2, 
                    GetScreenHeight()/2 - tex.height/2, 
                    WHITE);
        EndDrawing();
        // Load chunks
        m_chunkLock.lock();
        loading = (World::genCirclePoints(m_renderDistance).size() != m_chunks.size());
        for (const auto& chunk: m_chunks) {
            if (!chunk->isValid()) {
                chunk->tryUploadMeshes();
                loading = true;
            }
        }
        m_chunkLock.unlock();
    }
    std::cout << "World loaded " << m_chunks.size() << " chunks." << std::endl;
}

bool World::isLoaded()
{
    m_chunkLock.lock();
    bool loading = (World::genCirclePoints(m_renderDistance).size() != m_chunks.size());
    for (const auto& chunk: m_chunks) {
        if (!chunk->isValid()) {
            loading = true;
        }
    }
    m_chunkLock.unlock();
    return loading;
}

void World::drawChunks()
{
    m_chunkLock.lock();
    for (const auto& chunk: m_chunks) {
        if (!chunk->isValid()) {
            chunk->tryUploadMeshes();
        }
        else {
            if (chunk->isVisible(m_player)) {
                chunk->drawMesh();
            }
        }
    }
    m_chunkLock.unlock();
}

void World::updatePlayer(double dt)
{
    Vector2 dXY = GetMouseDelta();
    m_player.changeYaw(dXY.x);
    m_player.changePitch(dXY.y);
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
        m_player.moveUp(dt);
    }
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        m_player.moveUp(-dt);
    }

    Ray ray = GetScreenToWorldRay({(float)GetScreenWidth()/2, (float)GetScreenHeight()/2}, m_player);
    Vector2 playerChunk = m_player.getChunk();
    m_chunkLock.lock();
    RayCollision closestCollision = {false, 0, {0, 0, 0}, {0, 0, 0}};
    BlockMesh* closestChunk = nullptr;
    for (auto& chunk : m_chunks) {
        if (Vector2Distance(chunk->getChunkLoc(), playerChunk) <= 2) {
            RayCollision collision = chunk->rayCollision(ray);
            if (collision.hit) {
                if (!closestCollision.hit || collision.distance < closestCollision.distance) {
                    closestCollision = collision;
                    closestChunk = chunk.get();
                }
            }
        }
    }
    if (closestCollision.hit && closestCollision.distance < 6.0f) {
        closestCollision.point -= closestCollision.normal * 0.01f; // Offset the collision point slightly to avoid rounding issues
        closestCollision.point.x = roundf(closestCollision.point.x);
        closestCollision.point.y = roundf(closestCollision.point.y);
        closestCollision.point.z = roundf(closestCollision.point.z);
        Vector3 localCoord = Vector3Subtract(closestCollision.point, closestChunk->getGlobalCoord(0));
        m_player.setTargetBlock(closestCollision.point);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            std::cout << "Removing block at: " << localCoord.x << ", " << localCoord.y << ", " << localCoord.z << std::endl;
            closestChunk->setBlock((int)localCoord.x, (int)localCoord.y, (int)localCoord.z, Block::AIR, true);
        }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            localCoord += closestCollision.normal; // Place block on the side of the clicked block
            closestChunk->setBlock((int)localCoord.x, (int)localCoord.y, (int)localCoord.z, Block::DIRT, true);
        }
    }
    else {
        m_player.setTargetBlock({-INFINITY, -INFINITY, -INFINITY});
    }
    m_chunkLock.unlock();
}

Player World::getPlayer()
{
    return m_player;
}

Vector3 World::getSpawn()
{
    return {BlockMesh::LENGTH/2, 66, BlockMesh::LENGTH/2};
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

void World::runChunkLoader()
{
    Vector2 playerChunk = {INFINITY, INFINITY}; // Initialize to an invalid location
    std::unordered_set<Vector2, Utils::Vector2Hash, Utils::Vector2Equal> chunkLocs;
    while (m_running) {
        chunkLocs = World::genCirclePoints(m_renderDistance);
        Vector2 temp = Utils::floorVector(Vector2{m_player.getLocation().x, m_player.getLocation().z}, BlockMesh::LENGTH) / BlockMesh::LENGTH;
        if (playerChunk.x != INFINITY && Vector2Equals(temp, playerChunk)) {
            // No change in player position, skip chunk loading
            continue;
        }
        playerChunk = temp;
        for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
            // If chunk is too far away, unload it
            Vector2 chunkLoc = (*it)->getChunkLoc() - playerChunk;
            if (chunkLocs.find(chunkLoc) == chunkLocs.end()) {
                m_chunkLock.lock();
                m_chunks.erase(it);
                m_chunkLock.unlock();
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
            std::unique_ptr<BlockMesh> chunk = std::make_unique<BlockMesh>(m_perlinNoise, Vector3{chunkLoc.x * BlockMesh::LENGTH, 0, chunkLoc.y * BlockMesh::LENGTH});
            m_chunkLock.lock();
            m_chunks.push_back(std::move(chunk));
            m_chunkLock.unlock();
        }
    }
}
