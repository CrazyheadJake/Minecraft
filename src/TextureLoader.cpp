#include "TextureLoader.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include "raymath.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

Material TextureLoader::s_material;
const fs::path TextureLoader::BLOCKTEXTURES_PATH = "assets/textures/blocks/";
const fs::path TextureLoader::BLOCKMODELS_PATH = "assets/models/blocks/";
const fs::path TextureLoader::TEXTURES_DIR = "assets/textures/";
const Texture* TextureLoader::s_texture;
int TextureLoader::s_numTextures;
std::unordered_map<BlockFace, Vector2> TextureLoader::s_textureLocations;


void TextureLoader::loadTextures()
{   
    createDirectories();

    std::vector<fs::directory_entry> files;
    // Iterate through the png files in the textures
    for (const auto& entry : fs::directory_iterator(BLOCKTEXTURES_PATH)) {
        if (entry.path().extension() == ".png") {
            files.push_back(entry);
        }
        else {
            std::cout << entry.path().extension() << std::endl;
        }
    }
    createAtlas(files);
    createMap(files);
    s_texture = &s_material.maps[0].texture;
    std::cout << "Size: " << files.size() << std::endl;
}

void TextureLoader::createDirectories() {
    // Create the gates directory if it doesn't exist
    if (!fs::exists(BLOCKTEXTURES_PATH) || !fs::is_directory(BLOCKTEXTURES_PATH)) {
        fs::create_directory(BLOCKTEXTURES_PATH);
    }
    if (!fs::exists(BLOCKMODELS_PATH) || !fs::is_directory(BLOCKMODELS_PATH)) {
        fs::create_directory(BLOCKMODELS_PATH);
    }
}

void TextureLoader::createAtlas(const std::vector<fs::directory_entry>& files) {
    Image image = GenImageChecked(BLOCK_SIZE, BLOCK_SIZE*(files.size()+1), 8, 8, PURPLE, BLACK);
    s_numTextures = 1;  // First texture is always the purple/black checked texture
    for (int i = 0; i < files.size(); i++) {
        s_numTextures++;
        const fs::directory_entry& f = files[i];
        Image temp = LoadImage(f.path().c_str());
        ImageDraw(&image, temp, {0, 0, BLOCK_SIZE, BLOCK_SIZE}, {0, (float)BLOCK_SIZE*(i+1), BLOCK_SIZE, BLOCK_SIZE}, WHITE);
        UnloadImage(temp);
    }
    ExportImage(image, "temp.png");

    Texture texture = LoadTextureFromImage(image);
    s_material = LoadMaterialDefault();
    s_material.maps[0].texture = texture;
    UnloadImage(image);
}
void TextureLoader::createMap(const std::vector<fs::directory_entry>& files) {
    std::vector<fs::directory_entry> models;
    for (const auto& entry: fs::directory_iterator(BLOCKMODELS_PATH)) {
        if (entry.path().extension() == ".json")
            models.push_back(entry);
        else
            continue;
    }
    std::unordered_map<std::string, int> fileIndices;
    for (int i = 0; i < files.size(); i++) {
        fileIndices[files[i].path().string()] = i+1;
    }


    for (const auto& entry: models) {
        std::ifstream model(entry.path());
        json data = json::parse(model, nullptr, false);
        if (data.is_discarded())
            continue;
        const std::string& filename = entry.path().filename().stem();
        Block block = Blocks::getBlock(filename);
        if (data.contains("default")) {
            setFace(block, Direction::UP, "default", fileIndices, data);
            setFace(block, Direction::DOWN, "default", fileIndices, data);
            setFace(block, Direction::NORTH, "default", fileIndices, data);
            setFace(block, Direction::SOUTH, "default", fileIndices, data);
            setFace(block, Direction::EAST, "default", fileIndices, data);
            setFace(block, Direction::WEST, "default", fileIndices, data);
        }
        if (data.contains("sides")) {
            setFace(block, Direction::NORTH, "sides", fileIndices, data);
            setFace(block, Direction::SOUTH, "sides", fileIndices, data);
            setFace(block, Direction::EAST, "sides", fileIndices, data);
            setFace(block, Direction::WEST, "sides", fileIndices, data);
        }
        if (data.contains("up")) {
            setFace(block, Direction::UP, "up", fileIndices, data);
        }
        if (data.contains("down")) {
            setFace(block, Direction::DOWN, "down", fileIndices, data);
        }
        if (data.contains("north")) {
            setFace(block, Direction::NORTH, "north", fileIndices, data);
        }
        if (data.contains("south")) {
            setFace(block, Direction::SOUTH, "south", fileIndices, data);
        }
        if (data.contains("east")) {
            setFace(block, Direction::EAST, "east", fileIndices, data);
        }
        if (data.contains("west")) {
            setFace(block, Direction::WEST, "west", fileIndices, data);
        } 
    }
}

void TextureLoader::setFace(Block block, Direction dir, const std::string& field, const std::unordered_map<std::string, int>& fileIndices, const json& data) {
    float yLocation = BLOCK_SIZE*fileIndices.at(TEXTURES_DIR.string() + (std::string)data.at(field));
    s_textureLocations[{block, dir}] = {0, yLocation};
}

void TextureLoader::unloadTextures()
{
    UnloadMaterial(s_material);
}

Vector2 TextureLoader::getTexCoord(Block block, Direction d, Vector2 corner)
{   
    BlockFace face = {block, d};
    if (!s_textureLocations.contains(face))
        return {corner.x, corner.y/s_numTextures};
    Vector2 textureLocation = s_textureLocations[face];
    textureLocation.x += corner.x * BLOCK_SIZE;
    // Need 1-corner.y because opengl expects (0, 0) to be bottom left, whereas raylib expects (0,0) to be top left
    textureLocation.y += (1 - corner.y) * BLOCK_SIZE;
    textureLocation.y /= s_texture->height;
    textureLocation.x /= s_texture->width;
    return textureLocation;
}
