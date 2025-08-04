#include "BlockMesh.h"
#include <vector>
#include "Block.h"
#include <string.h>
#include "raymath.h"
#include <iostream>
#include <climits>
#include <span>
#include <algorithm>
#include "TextureLoader.h"
#include "Player.h"
#include "VectorUtils.h"
#include "Direction.h"
#include "rlgl.h"
#include "Structure.h"

BlockMesh::BlockMesh(World &world, const siv::PerlinNoise::seed_type seed, Vector3 offset) : m_chunkOffset(offset), m_world(world)
{
    generateWorld(seed);
    m_world.regenerateChunk(getChunkLoc() + Vector2{1, 0});
    m_world.regenerateChunk(getChunkLoc() + Vector2{-1, 0});
    m_world.regenerateChunk(getChunkLoc() + Vector2{0, 1});
    m_world.regenerateChunk(getChunkLoc() + Vector2{0, -1});

    m_boundingBox.min = getGlobalCoord(0);
    m_boundingBox.max = getGlobalCoord(LENGTH * LENGTH * HEIGHT - 1) + Vector3{1.0f, 1.0f, 1.0f};
}

BlockMesh::~BlockMesh()
{
    clearMeshData();
    clearMeshes();
    clearModel();
}

void BlockMesh::drawMesh() const
{
    if (m_state != State::MESH_UPLOADED)
        return;

    for (int i = 0; i < m_model.meshCount - m_translucentMeshCount; i++) {
        if (m_model.meshes[i].vertexCount > 0) {
            DrawMesh(m_model.meshes[i], m_model.materials[0], m_model.transform);
        }
    }

    DrawBoundingBox(m_boundingBox, RED);
}

void BlockMesh::drawTransparentMesh() const
{
    if (m_state != State::MESH_UPLOADED)
        return;
    for (int i = m_model.meshCount - m_translucentMeshCount; i < m_model.meshCount; i++) {
        if (m_model.meshes[i].vertexCount == 0)
            return;
        BeginBlendMode(BLEND_ALPHA);
        rlDisableBackfaceCulling();
        // rlDisableDepthMask();
        DrawMesh(m_model.meshes[i], m_model.materials[0], m_model.transform);
        // rlEnableDepthMask();
        rlEnableBackfaceCulling();
        EndBlendMode();
    }

}

void BlockMesh::generateMeshData()
{
    clearMeshData();

    for (int i = 0; i < m_blocks.size(); i++) {
        if (m_blocks[i] == Blocks::AIR)
            continue;
        genBlockData(getLocalCoord(i));
    }
    m_state = State::MESHDATA_GENERATED;
    m_regenerate = false;
}

void BlockMesh::generateMeshesFromData()
{
    int transparentMeshCount = ceilf((float)m_translucentVerticesCount / (MAX_VERTS + 1));
    int meshCount = ceilf((float)m_verticesCount / (MAX_VERTS + 1)) + transparentMeshCount;
    
    Mesh* meshes = (Mesh*)malloc(meshCount * sizeof(Mesh));
    m_translucentBlocks.clear();
    
    size_t verticesRemaining = m_verticesCount;
    auto it = m_meshData.begin();
    for (int i = 0; i < meshCount - transparentMeshCount; i++) {
        size_t size = verticesRemaining > MAX_VERTS ? MAX_VERTS : verticesRemaining;
        size_t indicesSize = size * 1.5f;
        float* verticesPtr = (float*)malloc(size * 3 * sizeof(float));
        float* texcoordsPtr = (float*)malloc(size * 2 * sizeof(float));
        float* normalsPtr = (float*)malloc(size * 3 * sizeof(float));
        unsigned short* indicesPtr = (unsigned short*)malloc(indicesSize * sizeof(unsigned short));
        meshes[i] = {0};
        meshes[i].triangleCount = indicesSize / 3;
        meshes[i].vertices = verticesPtr;
        meshes[i].texcoords = texcoordsPtr;
        meshes[i].normals = normalsPtr;
        meshes[i].indices = indicesPtr;
        
        const BlockData* blockData;
        int k = 0;
        for (; it != m_meshData.end(); it++) {
            blockData = &it->second;
            if (blockData->translucent){
                m_translucentBlocks.push_back(blockData);
                continue;
            }
            if (k + blockData->vertices.size() > size)
                break;
            memcpy(meshes[i].vertices + k * 3, blockData->vertices.data(), blockData->vertices.size() * sizeof(Vector3));
            memcpy(meshes[i].texcoords + k * 2, blockData->texcoords.data(), blockData->texcoords.size() * sizeof(Vector2));
            memcpy(meshes[i].normals + k * 3, blockData->normals.data(), blockData->normals.size() * sizeof(Vector3));
            for (int index = 0; index < blockData->indices.size(); index++) {
                meshes[i].indices[index + (int)(k * 1.5f)] = blockData->indices[index] + k;
            }
            
            k += blockData->vertices.size();
        }
        meshes[i].vertexCount = k;
        indicesSize = k * 1.5f;
        meshes[i].triangleCount = indicesSize / 3;
        verticesRemaining -= k;

    }
    
    generateTransparentMeshes(m_world.getPlayer().getLocation(), &meshes[meshCount - transparentMeshCount], meshCount, transparentMeshCount, true);
    
    if (m_model.meshCount > 0)
        clearMeshes();
    m_model.meshes = meshes;
    m_model.meshCount = meshCount;
    m_translucentMeshCount = transparentMeshCount;

    m_state = State::MESH_GENERATED;
}

void BlockMesh::updateTransparentMeshes(Vector3 location, bool newMesh)
{
    if (m_translucentBlocks.size() == 0)
        return;
    generateTransparentMeshes(location, &m_model.meshes[m_model.meshCount - m_translucentMeshCount], m_model.meshCount, m_translucentMeshCount, false);
    for (int i = 0; i < m_translucentMeshCount; i++) {
        Mesh& mesh = m_model.meshes[i + m_model.meshCount - m_translucentMeshCount];
        UpdateMeshBuffer(mesh, TextureLoader::MESH_BUFFER_VERTEX, mesh.vertices, mesh.vertexCount * sizeof(Vector3), 0);
        UpdateMeshBuffer(mesh, TextureLoader::MESH_BUFFER_TEXCOORD, mesh.texcoords, mesh.vertexCount * sizeof(Vector2), 0);
        UpdateMeshBuffer(mesh, TextureLoader::MESH_BUFFER_NORMAL, mesh.normals, mesh.vertexCount * sizeof(Vector3), 0);
    } 
}

void BlockMesh::generateTransparentMeshes(Vector3 location, Mesh* meshes, int meshCount, int transparentMeshCount, bool newMesh)
{
    std::sort(m_translucentBlocks.begin(), m_translucentBlocks.end(), [location](const BlockData* a, const BlockData* b) {
        return Vector3LengthSqr(a->location - location) > Vector3LengthSqr(b->location - location);
    });
    
    auto transparent_it = m_translucentBlocks.begin();
    size_t transparentVertsRemaining = m_translucentVerticesCount;
    for (int i = 0; i < transparentMeshCount; i++) {
        size_t size = transparentVertsRemaining > MAX_VERTS ? MAX_VERTS : transparentVertsRemaining;
        size_t indicesSize = size * 1.5f;
        float* texcoordsPtr;
        float* verticesPtr;
        float* normalsPtr;
        unsigned short* indicesPtr;
        if (newMesh) {
            texcoordsPtr = (float*)malloc(size * 2 * sizeof(float));
            verticesPtr = (float*)malloc(size * 3 * sizeof(float));
            normalsPtr = (float*)malloc(size * 3 * sizeof(float));
            indicesPtr = (unsigned short*)malloc(indicesSize * sizeof(unsigned short));

        }
        else {
            texcoordsPtr = meshes[i].texcoords;
            verticesPtr = meshes[i].vertices;
            normalsPtr = meshes[i].normals;
            indicesPtr = meshes[i].indices;
        }
        if (newMesh)
            meshes[i] = {0};
        meshes[i].vertices = verticesPtr;
        meshes[i].texcoords = texcoordsPtr;
        meshes[i].normals = normalsPtr;
        meshes[i].indices = indicesPtr;
        
        const BlockData* blockData;
        int k = 0;
        for (; transparent_it != m_translucentBlocks.end(); transparent_it++) {
            blockData = *transparent_it;
            if (k + blockData->vertices.size() > size)
                break;
            std::vector<int> keys(blockData->vertices.size() / 4);
            std::iota(keys.begin(), keys.end(), 0);
            std::sort(keys.begin(), keys.end(), [location, blockData](const int& a, const int& b) {
                Vector3 corner1a = blockData->vertices[4 * a];
                Vector3 corner1b = blockData->vertices[4 * a + 1];
                Vector3 corner1c = blockData->vertices[4 * a + 2];
                Vector3 corner1d = blockData->vertices[4 * a + 3];
                Vector3 corner1 = (corner1a + corner1b + corner1c + corner1d) / 4.0f;
                Vector3 corner2a = blockData->vertices[4 * b];
                Vector3 corner2b = blockData->vertices[4 * b + 1];
                Vector3 corner2c = blockData->vertices[4 * b + 2];
                Vector3 corner2d = blockData->vertices[4 * b + 3];
                Vector3 corner2 = (corner2a + corner2b + corner2c + corner2d) / 4.0f;
                return Vector3LengthSqr(corner1 - location) > Vector3LengthSqr(corner2 - location);
            });
            for (int index = 0; index < keys.size(); index++) {
                int key = keys[index];
                memcpy(meshes[i].vertices + (k * 3) + (12 * index), &blockData->vertices.data()[4 * key], 4 * sizeof(Vector3));
                memcpy(meshes[i].texcoords + (k * 2) + (8 * index), &blockData->texcoords.data()[4 * key], 4 * sizeof(Vector2));
                memcpy(meshes[i].normals + (k * 3) + (12 * index), &blockData->normals.data()[4 * key], 4 * sizeof(Vector3));
            }

            if (newMesh) {
                for (int index = 0; index < blockData->indices.size(); index++) {
                    meshes[i].indices[index + (int)(k * 1.5f)] = blockData->indices[index] + k;
                }
            }

            
            k += blockData->vertices.size();
        }
        meshes[i].vertexCount = k;
        indicesSize = k * 1.5f;
        meshes[i].triangleCount = indicesSize / 3;
        transparentVertsRemaining -= k;
    }
}

bool BlockMesh::shouldRegenerate() const
{
    return m_regenerate;
}

void BlockMesh::clearMeshData() {
    m_meshData.clear();
    m_translucentBlocks.clear();
    m_verticesCount = 0;
    m_translucentVerticesCount = 0;
}


void BlockMesh::clearMeshes()
{
    for (int mesh = 0; mesh < m_model.meshCount; mesh++) {
        bool valid = IsModelValid(m_model);
        UnloadMesh(m_model.meshes[mesh]);
    }
    free(m_model.meshes);
    m_model.meshCount = 0;
}

void BlockMesh::clearModel()
{
    m_model.materialCount = 0;
    free(m_model.materials);
    free(m_model.meshMaterial);
}

void BlockMesh::updateBlockData(Vector3 localCoord)
{
    int i = (int)(localCoord.x) + (int)(localCoord.y) * LENGTH * LENGTH + (int)(localCoord.z) * LENGTH;
    auto it = m_meshData.find(i);
    if (it != m_meshData.end()) {
        if (it->second.translucent)
            m_translucentVerticesCount -= it->second.vertices.size();
        else
            m_verticesCount -= it->second.vertices.size();
        m_meshData.erase(it);
    }
    if (m_blocks[i] != Blocks::AIR) {
        genBlockData(localCoord);
    }
    m_state = State::MESHDATA_GENERATED;
}

void BlockMesh::genBlockData(Vector3 localCoord)
{
    int i = (int)(localCoord.x) + (int)(localCoord.y) * LENGTH * LENGTH + (int)(localCoord.z) * LENGTH;

    const Vector3 offset = localCoord + m_chunkOffset;
    const std::vector<Vector3>& blockVertices = m_blocks[i].getVertices();
    const std::vector<unsigned short>& blockIndices = m_blocks[i].getIndices();
    const std::vector<Vector2>& blockTexcoords = m_blocks[i].getTexcoords();
    const std::vector<Vector3>& blockNormals = m_blocks[i].getNormals();

    std::pair<std::unordered_map<int, BlockMesh::BlockData>::iterator, bool> insertion;
    int facesSkipped = 0;
    BlockData* blockData;
    localCoord += Vector3 {0.5f, 0.5f, 0.5f};
    for (int face = 0; face < m_blocks[i].getVertices().size() / 4; face++) {
        Vector3 normal = blockNormals[face * 4];
        Vector3 faceRelativeCenter = (blockVertices[face * 4] + 
            blockVertices[face * 4 + 1] + 
            blockVertices[face * 4 + 2] + 
            blockVertices[face * 4 + 3]) / 4.0f;
        Vector3 faceCenter = localCoord + faceRelativeCenter;
        Block neighborBlock = getBlockLocal(faceCenter + normal * 0.01f);
        // If neighbor block is in a different chunk, check the neighboring chunks
        if (neighborBlock == Blocks::UNKNOWN) {
            neighborBlock = m_world.getBlockGlobal(localCoord + m_chunkOffset + normal);
        }

        // If neighbor block is transparent or translucent, force the rendering of the face, unless its the same as the current block
        bool forceFace = false;
        if (neighborBlock.getTransparent())
            forceFace = true;
        if (neighborBlock.getTranslucent() && neighborBlock != m_blocks[i])
            forceFace = true;
        if (!forceFace && (neighborBlock != Blocks::AIR || neighborBlock == Blocks::UNKNOWN)) {
            facesSkipped++;
            continue; 
        }
        // If the block data doesn't exist yet (ie this is the first face), generate block data
        if (m_meshData.find(i) == m_meshData.end()) {
            insertion = m_meshData.insert_or_assign(i, BlockData {});
            blockData = &insertion.first->second;
            blockData->translucent = m_blocks[i].getTranslucent();
            blockData->block = m_blocks[i];
            blockData->location = localCoord + m_chunkOffset;
        }
        // Insert block info into block data structure
        for (int index = face * 6; index < face * 6 + 6; index++) {
            blockData->indices.push_back(blockIndices[index] - facesSkipped * 4);
        }
        for (int vert = face * 4; vert < face * 4 + 4; vert++) {
            blockData->vertices.push_back(blockVertices[vert] + offset);
            Direction d = Dir::getDirection(blockNormals[vert]);
            blockData->texcoords.push_back(TextureLoader::getTexCoord(m_blocks[i], d, blockTexcoords[vert]));
            blockData->normals.push_back(blockNormals[vert]);
        }
        // Keep track of how many vertices we have
        if (m_blocks[i].getTranslucent())
            m_translucentVerticesCount += 4;
        else
            m_verticesCount += 4;
    }
}

void BlockMesh::requestRegenerate()
{
    m_regenerate = true;
}

void BlockMesh::generateWorld(const siv::PerlinNoise::seed_type seed)
{
    siv::PerlinNoise perlin(seed);

    for (int x  = 0; x < LENGTH; x++) {
        for (int z = 0; z < LENGTH; z++) {    
            double noise = perlin.octave2D((double)(x + m_chunkOffset.x) * SCALE, (double)(z + m_chunkOffset.z) * SCALE, 3, 0.5);
            int topHeight = SEALEVEL + (int)(noise * 20);
            for (int y = 0; y < HEIGHT; y++) {
                int priority = getBlockLocal(Vector3{(float)x, (float)y, (float)z}).getGenerationPriority();
                if (y > topHeight && y > SEALEVEL) {
                    if (Blocks::AIR.getGenerationPriority() > priority)
                        setBlock(x, y, z, Blocks::AIR);
                }
                else if (y > topHeight && y <= SEALEVEL) {
                    if (Blocks::WATER.getGenerationPriority() > priority)
                        setBlock(x, y, z, Blocks::WATER);
                }
                else if (y == topHeight) {
                    if (Blocks::GRASS_BLOCK.getGenerationPriority() > priority)
                        setBlock(x, y, z, Blocks::GRASS_BLOCK);
                }
                else if (y > topHeight - 4) {
                    if (Blocks::DIRT.getGenerationPriority() > priority)
                        setBlock(x, y, z, Blocks::DIRT);
                }
                else if (Blocks::STONE.getGenerationPriority() > priority)
                    setBlock(x, y, z, Blocks::STONE);
            }
            // Tree generation
            int s = (x + (int)m_chunkOffset.x) ^ 1402978341 + (z + (int)m_chunkOffset.z) * 1243 + seed;
            srand(s);
            double treeValue = rand() / (double)RAND_MAX;
            if (treeValue < 0.02 && topHeight >= SEALEVEL) {
                int random = rand();
                int index = random % Structures::TREE.getStructureCount();
                std::vector<BlockInstance> tree = Structures::TREE.getStructure(index);
                for (int i = 0; i < tree.size(); i++) {
                    Vector3 loc = Vector3 {(float)x, (float)topHeight + 1, (float)z} + tree[i].location;
                    // Block is in the chunk
                    if (isLocalCoord(loc)) {
                        if (getBlockLocal(loc).getGenerationPriority() >= tree[i].block.getGenerationPriority())
                            continue;
                        setBlock(loc, tree[i].block);
                    }
                    // Block is in a neighboring chunk
                    else {
                        if (m_world.getBlockGlobal(loc + m_chunkOffset).getGenerationPriority() >= tree[i].block.getGenerationPriority())
                            continue;
                        tree[i].location = loc + m_chunkOffset;
                        // NOTE - because updateMesh is false, there will be a couple extra (nonvisible) faces in the mesh
                        // When it is set to true, we get a bug where some chunks won't load
                        m_world.setBlockGlobal(tree[i].location, tree[i].block, false);
                    }
                }
            }
            // Foliage generation
            double foliageValue = rand() / (double)RAND_MAX;
            if (foliageValue < 0.06 && topHeight >= SEALEVEL) {
                if (Blocks::GRASS.getGenerationPriority() > getBlockLocal({(float)x, (float)topHeight + 1, (float)z}).getGenerationPriority())
                    setBlock({(float)x, (float)topHeight + 1, (float)z}, Blocks::GRASS);
            }

        }
    }

    std::vector<BlockInstance> futureBlocks = m_world.getFutureBlocks(getChunkLoc());
    for (BlockInstance block : futureBlocks) {
        setBlock(block.location - m_chunkOffset, block.block);
    }

    m_state = State::WORLD_GENERATED;
}

bool BlockMesh::isValid() const
{
    return m_state == State::MESH_UPLOADED;
}

bool BlockMesh::isVisible(Player &camera) const
{
    // float maxAngle = Utils::toRadians(camera.getFov()) / sqrt(2);
    for (int i = 0; i < 8; i++) {
        Vector3 corner = getCorner(i);
        // if (Vector3Angle(corner - camera.getLocation(), camera.getDirection()) < maxAngle)
        //     return true;
        if (Vector3DotProduct(corner - camera.getLocation(), camera.getDirection()) > 0)
            return true;
    }
    return false;
}

void BlockMesh::tryUploadMeshes()
{
    if (m_state == State::MESH_GENERATED) {
        uploadMeshes();
    }
}

Vector3 BlockMesh::getLocalCoord(int i) const
{
    return Vector3{(float)(i % LENGTH),
        (float)((i / LENGTH / LENGTH) % HEIGHT), 
        (float)((i / LENGTH) % LENGTH)};
}

void BlockMesh::uploadMeshes()
{
    if (m_model.materialCount == 0) {
        generateModel();
    }
    for (int mesh = 0; mesh < m_model.meshCount - m_translucentMeshCount; mesh++) {
        UploadMesh(&m_model.meshes[mesh], false);
    }
    for (int mesh = m_model.meshCount - m_translucentMeshCount; mesh < m_model.meshCount; mesh++) {
        UploadMesh(&m_model.meshes[mesh], true);
    }
    m_state = State::MESH_UPLOADED;
    m_boundingBox = GetModelBoundingBox(m_model);
}

int BlockMesh::getIndex(Vector3 coord)
{
    return coord.x + coord.y * LENGTH * LENGTH + coord.z * LENGTH;
}

Vector3 BlockMesh::getGlobalCoord(int i) const
{
    return Vector3Add({(float)(i % LENGTH),
        (float)((i / LENGTH / LENGTH) % HEIGHT), 
        (float)((i / LENGTH) % LENGTH)}, m_chunkOffset);
}

Vector2 BlockMesh::getChunkLoc() const
{
    return {floorf(m_chunkOffset.x/LENGTH), floorf(m_chunkOffset.z/LENGTH)};
}

void BlockMesh::setBlock(Vector3 loc, Block block, bool updateMesh)
{
    setBlock((int)loc.x, (int)loc.y, (int)loc.z, block, updateMesh);
}

void BlockMesh::setBlock(int x, int y, int z, Block block, bool updateMesh)
{
    m_blocks[z * LENGTH + y * LENGTH * LENGTH + x] = block;
    if (updateMesh) {
        lock();
        updateBlockData({(float)x, (float)y, (float)z});
        unlock();
    }
}

Block BlockMesh::getBlockLocal(Vector3 localCoord) const
{
    if (localCoord.x < 0 || localCoord.x >= LENGTH ||
        localCoord.y < 0 || localCoord.y >= HEIGHT ||
        localCoord.z < 0 || localCoord.z >= LENGTH) {
        return Blocks::UNKNOWN;
    }
    return m_blocks[(int)(localCoord.x) + (int)(localCoord.y) * LENGTH * LENGTH + (int)(localCoord.z) * LENGTH];
}

bool BlockMesh::isLocalCoord(Vector3 localCoord) const
{
    return localCoord.x >= 0 && localCoord.x < LENGTH &&
           localCoord.y >= 0 && localCoord.y < HEIGHT &&
           localCoord.z >= 0 && localCoord.z < LENGTH;
}

void BlockMesh::tryGenerateMeshData() {
    if (m_state == State::WORLD_GENERATED || m_regenerate) {
        generateMeshData();
    }
}

void BlockMesh::tryGenerateMeshes()
{
    if (m_state == State::MESHDATA_GENERATED) {
        generateMeshesFromData();
    }
}

Vector3 BlockMesh::getCorner(int i) const
{
    switch (i) {
        case 0: return m_boundingBox.min;
        case 1: return {m_boundingBox.max.x, m_boundingBox.min.y, m_boundingBox.min.z};
        case 2: return {m_boundingBox.min.x, m_boundingBox.min.y, m_boundingBox.max.z};
        case 3: return {m_boundingBox.max.x, m_boundingBox.min.y, m_boundingBox.max.z};
        case 4: return {m_boundingBox.min.x, m_boundingBox.max.y, m_boundingBox.min.z};
        case 5: return {m_boundingBox.max.x, m_boundingBox.max.y, m_boundingBox.min.z};
        case 6: return {m_boundingBox.min.x, m_boundingBox.max.y, m_boundingBox.max.z};
        case 7: return m_boundingBox.max;
    }
    return {0, 0, 0}; // This line is unreachable, but it prevents a warning about not returning a value
}

void BlockMesh::generateModel()
{
    m_model.transform = MatrixTranslate(0.5, 0.5, 0.5); // Block corners are now integers instead of 0.5
    m_model.materialCount = 1;
    m_model.materials = (Material*)malloc(sizeof(Material));
    m_model.meshMaterial = (int*)malloc(sizeof(int));
    m_model.meshMaterial[0] = 0;

    // I'm afraid of memory leaks here (maybe GPU), but address sanitizer seems to think there are none
    m_model.materials[0] = TextureLoader::s_material;

    m_boundingBox = GetModelBoundingBox(m_model);
}

void BlockMesh::lock()
{
    m_dataLock.lock();
}

void BlockMesh::unlock()
{
    m_dataLock.unlock();
}