#ifndef ASSETMANAGER_HH
#define ASSETMANAGER_HH

#include <array>
#include <cstddef>
#include <raylib.h>

enum class TextureId 
{
    TilesetGround,
    TilesetSlope,
	Count // Last element to keep track of the number of textures
};

class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    const Texture2D& GetTexture(TextureId id) const;

    static AssetManager* instance;

private:
    void LoadTextures();
    void UnloadTextures();

    std::array<Texture2D, (size_t)TextureId::Count> _textures{};
};

#endif