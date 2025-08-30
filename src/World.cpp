#include "World.h"
#include "raymath.h"
#include "rlgl.h"
#include <unordered_set>
#include "VectorUtils.h"
#include <iostream>
#include <thread>
#include "TextureLoader.h"

World::World(int seed): m_seed(seed), m_perlinNoise(m_seed), m_chunkLoader(&World::runChunkLoader, this), m_player(getSpawn(), 90.0f, CAMERA_PERSPECTIVE)
{
    // Generate the skybox
    m_skybox = Blocks::DIRT.generateModel();
    m_skybox.materials[0].shader = LoadShader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
    for (int i = 0; i < m_skybox.meshes->vertexCount; i++) {
        m_skybox.meshes->vertices[3*i] *= 1000;
        m_skybox.meshes->vertices[3*i+1] *= 1000;
        m_skybox.meshes->vertices[3*i+2] *= 1000;
    }
    UploadMesh(&m_skybox.meshes[0], false);

    // Sun and moon textures
    m_sun = LoadModelFromMesh(GenMeshPlane(150, 150, 1, 1));
    m_moon = LoadModelFromMesh(GenMeshPlane(150, 150, 1, 1));

    m_sun.materials[0].shader = LoadShader("assets/shaders/sunmoon.vs", "assets/shaders/sunmoon.fs");
    m_moon.materials[0].shader = m_sun.materials[0].shader;

    m_sun.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("assets/textures/sprites/sun.png");
    m_moon.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("assets/textures/sprites/moon.png");
}

World::~World() {
    // End the chunk loader thread
    m_running = false;
    if (m_chunkLoader.joinable())
        m_chunkLoader.join();

    // Free skybox
    UnloadMesh(m_skybox.meshes[0]);
    free(m_skybox.meshes);
    free(m_skybox.materials);
    free(m_skybox.meshMaterial);

    // Sun and moon
    UnloadModel(m_sun);
    UnloadModel(m_moon);
}

void World::update(double dt)
{
    static Vector2 playerChunk;
    if (IsKeyPressed(KEY_F2)) {
        EnableCursor();
        std::cout << "DEBUGGING" << std::endl;
    }

    updatePlayer(dt);

    playerChunk = Utils::floorVector(Vector2{m_player.getLocation().x, m_player.getLocation().z}, BlockMesh::LENGTH) / BlockMesh::LENGTH;
    sortChunks(playerChunk);

    m_time = GetTime() / 1000.0f;
    float time = getDayTime();
    SetShaderValue(TextureLoader::s_material.shader, TIMELOC, &time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_skybox.materials[0].shader, TIMELOC, &time, SHADER_UNIFORM_FLOAT);
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
        loading = (World::genSquarePoints(m_renderDistance).size() != m_chunks.size());
        for (const auto& [chunkLoc, chunk]: m_chunks) {
            if (!chunk->isValid()) {
                m_chunkLock.unlock();
                chunk->tryGenerateMeshes();
                chunk->tryUploadMeshes();
                loading = true;
                m_chunkLock.lock();
            }
        }
        m_chunkLock.unlock();
    }
    std::cout << "World loaded " << m_chunks.size() << " chunks." << std::endl;
}

void World::drawChunks()
{
    static Vector3 lastPlayerLoc;
    std::vector<std::reference_wrapper<BlockMesh>> chunks_to_draw;
    m_chunkLock.lock();
    chunks_to_draw = m_sortedChunks;
    m_chunkLock.unlock();
    int chunksDrawn = 0;
    // Drawing non-transparent blocks for each chunk
    bool updateTranslucentMeshes = Utils::floorVector(m_player.getLocation()) != lastPlayerLoc;
    for (const auto& chunkRef: chunks_to_draw) {
        BlockMesh& chunk = chunkRef.get();
        chunk.tryGenerateMeshes();
        chunk.tryUploadMeshes();
        if (updateTranslucentMeshes && chunk.isValid())
            chunk.updateTransparentMeshes(m_player.getLocation());

        if (chunk.isVisible(m_player) && chunk.isValid()) {
            chunk.drawMesh();
            chunk.drawTransparentMesh();

            chunksDrawn++;
        }
    }

    lastPlayerLoc = Utils::floorVector(m_player.getLocation());
}

void World::drawSkybox()
{   
    Vector3 pos = m_player.getLocation();
    rlDisableBackfaceCulling();  // This will render both sides of the skybox so it appears to the player

    DrawMesh(m_skybox.meshes[0], m_skybox.materials[0], MatrixTranslate(pos.x, pos.y, pos.z));
    rlEnableBackfaceCulling();  // Restore culling

    Vector3 sunPos = Vector3{0, -250, 0};
    Vector3 moonPos = Vector3{0, -250, 0};
    printf("Time: %f\n", m_time - (int)m_time);
    DrawMesh(m_sun.meshes[0], m_sun.materials[0], 
        MatrixMultiply(MatrixTranslate(sunPos.x, sunPos.y, sunPos.z), MatrixMultiply(MatrixRotateX(-getDayTime() * 2 * M_PI), MatrixTranslate(pos.x, pos.y, pos.z))));
    DrawMesh(m_moon.meshes[0], m_moon.materials[0], 
        MatrixMultiply(MatrixTranslate(moonPos.x, moonPos.y, moonPos.z), MatrixMultiply(MatrixRotateX(-getDayTime() * 2 * M_PI + M_PI), MatrixTranslate(pos.x, pos.y, pos.z))));
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
        m_player.moveRight(dt);
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

    for (int i = 0; i < 9; i++) {
        if (IsKeyDown(KEY_ONE + i)) {
            std::cout << "Key " << i + 1 << " pressed" << std::endl;
            m_player.setHeldBlock(i + 3);
        }
    }


    Ray ray = GetScreenToWorldRay({(float)GetScreenWidth()/2, (float)GetScreenHeight()/2}, m_player);
    RayCollision collision = rayCollision(ray, 6.0f);

    if (collision.hit && collision.distance <= 6.0f) {
        m_player.setTargetBlock(collision.point);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            setBlockGlobal(collision.point, Blocks::AIR, true);
        }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            collision.point += collision.normal; // Place block on the side of the clicked block
            setBlockGlobal(collision.point, m_player.getHeldBlock(), true);
        }
    }
    else {
        m_player.setTargetBlock({-INFINITY, -INFINITY, -INFINITY});
    }
}

const Player& World::getPlayer() const
{
    return m_player;
}

Vector3 World::getSpawn() const
{
    return {BlockMesh::LENGTH/2, 66, BlockMesh::LENGTH/2};
}

RayCollision World::rayCollision(const Ray &ray, float distance) const
{
    Vector3 start = ray.position;
    int stepX = (ray.direction.x > 0) ? 1 : -1;
    int stepY = (ray.direction.y > 0) ? 1 : -1;
    int stepZ = (ray.direction.z > 0) ? 1 : -1;
    float tDeltaX = fabs(1.0f / ray.direction.x);
    float tDeltaY = fabs(1.0f / ray.direction.y);
    float tDeltaZ = fabs(1.0f / ray.direction.z);
    float tMaxX = (stepX > 0) ? (floorf(start.x + 1) - start.x) * tDeltaX : (start.x - floorf(start.x)) * tDeltaX;
    float tMaxY = (stepY > 0) ? (floorf(start.y + 1) - start.y) * tDeltaY : (start.y - floorf(start.y)) * tDeltaY;
    float tMaxZ = (stepZ > 0) ? (floorf(start.z + 1) - start.z) * tDeltaZ : (start.z - floorf(start.z)) * tDeltaZ;
    if (tDeltaX == INFINITY) tMaxX = INFINITY;
    if (tDeltaY == INFINITY) tMaxY = INFINITY;
    if (tDeltaZ == INFINITY) tMaxZ = INFINITY;
    int lastStep = 0;

    while (Vector3DistanceSqr(start, ray.position) <= distance * distance) {
        if(tMaxX < tMaxY) {
            if(tMaxX < tMaxZ) {
                start.x += stepX;
                tMaxX += tDeltaX;
                lastStep = 0; // X step
            } 
            else {
                start.z += stepZ;
                tMaxZ += tDeltaZ;
                lastStep = 2; // Z step
            }
        } 
        else {
            if(tMaxY < tMaxZ) {
                start.y += stepY;
                tMaxY += tDeltaY;
                lastStep = 1; // Y step
            } 
            else {
                start.z += stepZ;
                tMaxZ += tDeltaZ;
                lastStep = 2; // Z step
            }
        }
        if (getBlockGlobal(start).block != Blocks::AIR) {
            // We hit a block, return the collision
            RayCollision collision;
            collision.hit = true;
            collision.distance = Vector3Distance(ray.position, start);
            collision.point = start;
            collision.normal = {0, 0, 0};
            if (lastStep == 0) {
                collision.normal.x = -stepX;
            } else if (lastStep == 1) {
                collision.normal.y = -stepY;
            } else if (lastStep == 2) {
                collision.normal.z = -stepZ;
            }
            return collision;
        }
    }

    return {false, 0, {0, 0, 0}, {0, 0, 0}};
}

BlockData World::getBlockGlobal(Vector3 globalCoord) const
{
    if (globalCoord.y < 0 || globalCoord.y >= BlockMesh::HEIGHT)
        return Blocks::AIR;  // Below world, will force bottom faces to render
    Vector2 chunkLoc = Utils::floorVector(Vector2{globalCoord.x, globalCoord.z}, BlockMesh::LENGTH) / BlockMesh::LENGTH;
    m_chunkLock.lock();
    auto it = m_chunks.find(chunkLoc);
    if (it == m_chunks.end()) {
        m_chunkLock.unlock();
        return Blocks::UNKNOWN; // Chunk not found
    }
    const std::unique_ptr<BlockMesh>& chunk = it->second;
    m_chunkLock.unlock();
    Vector3 localCoord = globalCoord - chunk->getGlobalCoord(0);
    return chunk->getBlockLocal(localCoord);
}

void World::setBlockGlobal(Vector3 globalCoord, Block block, bool updateMesh)
{
    if (globalCoord.y < 0 || globalCoord.y >= BlockMesh::HEIGHT)
        return;  // Below/above world
    globalCoord = Utils::floorVector(globalCoord);
    Vector2 chunkLoc = Utils::floorVector(Vector2{globalCoord.x, globalCoord.z}, BlockMesh::LENGTH) / BlockMesh::LENGTH;

    // Add the block to future blocks
    auto future_it = m_futureBlocks.find(chunkLoc);
    if (future_it == m_futureBlocks.end()) {
        auto insertion = m_futureBlocks.insert_or_assign(chunkLoc, std::vector<BlockInstance>{});
        insertion.first->second.push_back({block, globalCoord});
    }
    else
        future_it->second.push_back({block, globalCoord});

    // Attempt to set the block if the chunk is loaded
    m_chunkLock.lock();
    auto it = m_chunks.find(chunkLoc);
    if (it == m_chunks.end()) {
        m_chunkLock.unlock();
        return; // Chunk not found
    }
    const std::unique_ptr<BlockMesh>& chunk = it->second;
    m_chunkLock.unlock();
    Vector3 localCoord = globalCoord - chunk->getGlobalCoord(0);
    chunk->setBlock(localCoord, block, updateMesh);
    
    // Update surrounding blocks for their meshes to be updated
    if (updateMesh) {
        updateBlockGlobal(globalCoord + Vector3{1.0f, 0.0f, 0.0f});
        updateBlockGlobal(globalCoord + Vector3{-1.0f, 0.0f, 0.0f});
        updateBlockGlobal(globalCoord + Vector3{0.0f, 1.0f, 0.0f});
        updateBlockGlobal(globalCoord + Vector3{0.0f, -1.0f, 0.0f});
        updateBlockGlobal(globalCoord + Vector3{0.0f, 0.0f, 1.0f});
        updateBlockGlobal(globalCoord + Vector3{0.0f, 0.0f, -1.0f});
        
        // Light update the current block
        updateLightGlobal(globalCoord);
    }
}

void World::updateBlockGlobal(Vector3 globalCoord)
{
    if (globalCoord.y < 0 || globalCoord.y >= BlockMesh::HEIGHT)
        return;  // Below/above world
    Vector2 chunkLoc = Utils::floorVector(Vector2{globalCoord.x, globalCoord.z}, BlockMesh::LENGTH) / BlockMesh::LENGTH;
    m_chunkLock.lock();
    auto it = m_chunks.find(chunkLoc);
    if (it == m_chunks.end()) {
        m_chunkLock.unlock();
        return; // Chunk not found
    }
    const std::unique_ptr<BlockMesh>& chunk = it->second;
    m_chunkLock.unlock();
    Vector3 localCoord = globalCoord - chunk->getGlobalCoord(0);
    chunk->lock();
    chunk->updateBlockData(localCoord);
    chunk->unlock();
}

void World::updateLightGlobal(Vector3 globalCoord)
{
    if (globalCoord.y < 0 || globalCoord.y >= BlockMesh::HEIGHT)
        return;  // Below/above world
    Vector2 chunkLoc = Utils::floorVector(Vector2{globalCoord.x, globalCoord.z}, BlockMesh::LENGTH) / BlockMesh::LENGTH;
    m_chunkLock.lock();
    auto it = m_chunks.find(chunkLoc);
    if (it == m_chunks.end()) {
        m_chunkLock.unlock();
        return; // Chunk not found
    }
    const std::unique_ptr<BlockMesh>& chunk = it->second;
    m_chunkLock.unlock();
    Vector3 localCoord = globalCoord - chunk->getGlobalCoord(0);
    // chunk->lock();
    chunk->updateLightData(localCoord);
    // chunk->unlock();
}

void World::regenerateChunk(Vector2 chunkLoc)
{
    m_chunkLock.lock();
    auto it = m_chunks.find(chunkLoc);
    // Chunk doesn't exist
    if (it == m_chunks.end()) {
        m_chunkLock.unlock();
        return;
    }
    // Reload mesh for existing chunk
    std::unique_ptr<BlockMesh>& chunk = it->second;
    m_chunkLock.unlock();
    chunk->requestRegenerate();
}

void World::relightChunk(Vector2 chunkLoc)
{
    m_chunkLock.lock();
    auto it = m_chunks.find(chunkLoc);
    // Chunk doesn't exist
    if (it == m_chunks.end()) {
        m_chunkLock.unlock();
        return;
    }
    // Reload mesh for existing chunk
    std::unique_ptr<BlockMesh>& chunk = it->second;
    m_chunkLock.unlock();
    chunk->generateLightData();
}

std::vector<BlockInstance> World::getFutureBlocks(Vector2 chunkLoc)
{
    auto it = m_futureBlocks.find(chunkLoc);
    if (it == m_futureBlocks.end()) {
        return {};
    }
    return it->second;
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

std::unordered_set<Vector2, Utils::Vector2Hash, Utils::Vector2Equal> World::genSquarePoints(float radius)
{
    std::unordered_set<Vector2, Utils::Vector2Hash, Utils::Vector2Equal> set = {};
    for (int i = 0; i <= radius; i++) {
        for (int k = 0; k <= radius; k++) {
            set.insert(Vector2{(float)i,(float)k});
            set.insert(Vector2{(float)i,(float)-k});
            set.insert(Vector2{(float)-i,(float)k});
            set.insert(Vector2{(float)-i,(float)-k});
        }
    }
    return set;
}

void World::runChunkLoader()
{
    Vector2 playerChunk = {INFINITY, INFINITY}; // Initialize to an invalid location
    std::unordered_set<Vector2, Utils::Vector2Hash, Utils::Vector2Equal> chunkLocs;
    while (m_running) {
        chunkLocs = World::genSquarePoints(m_renderDistance);
        Vector2 temp = Utils::floorVector(Vector2{m_player.getLocation().x, m_player.getLocation().z}, BlockMesh::LENGTH) / BlockMesh::LENGTH;
        if (playerChunk.x != INFINITY && Vector2Equals(temp, playerChunk) && chunkLocs.size() == m_chunks.size()) {
            // No change in player position, skip chunk loading
            continue;
        }
        playerChunk = temp;

        // CAREFUL, we are not locking before iterating over m_chunks, so we must ensure no other thread is invalidating iterators
        for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
            // If chunk is too far away, unload it
            Vector2 chunkLoc = it->second->getChunkLoc() - playerChunk;
            if (chunkLocs.find(chunkLoc) == chunkLocs.end()) {
                m_chunkLock.lock();
                // Get the chunk to be unloaded, but don't unload it until after the lock is released
                std::unique_ptr<BlockMesh> chunk = std::move(it->second);   
                it = m_chunks.erase(it);
                for (auto sortedIt = m_sortedChunks.begin(); sortedIt != m_sortedChunks.end(); sortedIt++) {
                    if (&sortedIt->get() == chunk.get()) {
                        m_sortedChunks.erase(sortedIt);
                        break;
                    }
                }
                m_chunkLock.unlock();
                // it++;
            }
            // Chunk is already loaded, so we can remove it from the set of chunk locations
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
        // All remaining chunk locations in chunkLocs are new chunks to be loaded
        for (Vector2 chunkLoc: chunkLocs) {
            chunkLoc += playerChunk; // Offset chunk location to player's position
            // Generate the chunk before we lock the mutex to avoid blocking other threads
            std::unique_ptr<BlockMesh> chunk = std::make_unique<BlockMesh>(*this, m_seed, Vector3{chunkLoc.x * BlockMesh::LENGTH, 0, chunkLoc.y * BlockMesh::LENGTH});
            m_chunkLock.lock();
            m_chunks.insert_or_assign(chunk->getChunkLoc(), std::move(chunk));
            m_chunkLock.unlock();
        }
        for (const auto& [chunkLoc, chunk]: m_chunks) {
            // m_chunkLock.lock();
            chunk->lock();
            chunk->tryGenerateMeshData();
            chunk->unlock();
            // m_chunkLock.unlock();
        }
    }
}

void World::sortChunks(Vector2 playerChunkLoc)
{
        // Sort chunks by distance to the player, needed for transparent block rendering
        m_chunkLock.lock();
        m_sortedChunks.clear();
        m_sortedChunks.reserve(m_chunks.size());
        for (const auto& [chunkLoc, chunk]: m_chunks) {
            m_sortedChunks.push_back(std::ref(*chunk));
        }
        std::sort(m_sortedChunks.begin(), m_sortedChunks.end(), [playerChunkLoc](const std::reference_wrapper<BlockMesh>& a, const std::reference_wrapper<BlockMesh>& b) {
            return Vector2LengthSqr(a.get().getChunkLoc() - playerChunkLoc) > Vector2LengthSqr(b.get().getChunkLoc() - playerChunkLoc);
        });
        m_chunkLock.unlock();
}

float World::getDayTime()
{
    return (m_time - (int)m_time);
}
