#pragma once
#include "nlohmann/json.hpp"
#include <unordered_map>
#include "Blocks.h"
#include <functional>

using json = nlohmann::json;

// Hash specialization for Key
namespace std {
    template <>
    struct hash<BlockFace> {
        size_t operator()(const BlockFace& k) const {
            // Hash the enums by casting to their underlying type
            int block = static_cast<int>(k.block);
            int face = static_cast<int>(k.face);
            // Combine the hashes
            return block ^ (face << 1);
        }
    };
}

namespace fs = std::filesystem;

class TextureLoader {
    public:
        static Material s_material;
        TextureLoader() = delete;
        static void loadTextures();
        static void unloadTextures();
        static Vector2 getTexCoord(Block block, Direction d, Vector2 corner);

    private:
        static const int BLOCK_SIZE = 16;
        static std::unordered_map<BlockFace, Vector2> s_textureLocations;
        static const fs::path BLOCKTEXTURES_PATH;
        static const fs::path BLOCKMODELS_PATH;
        static const fs::path TEXTURES_DIR;

        static const Texture* s_texture;
        static int s_numTextures;

        static void setFace(Block block, Direction d, const std::string& field, const std::unordered_map<std::string, int>& fileIndices, const json& data);
        static void createDirectories();
        static void createMap(const std::vector<fs::directory_entry>& files);
        static void createAtlas(const std::vector<fs::directory_entry>& files);
};