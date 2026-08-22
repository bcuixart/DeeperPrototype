#include "AssetManager.hh"

AssetManager* AssetManager::instance = nullptr;

AssetManager::AssetManager()
{
	AssetManager::instance = this;

	LoadTextures();
}

AssetManager::~AssetManager()
{
	AssetManager::instance = nullptr;

	UnloadTextures();
}

void AssetManager::LoadTextures()
{
	_textures[(size_t)TextureId::TilesetGround] = LoadTexture("assets/sprites/tilesets/Tileset_Grass.png");
	_textures[(size_t)TextureId::TilesetSlope] = LoadTexture("assets/sprites/tilesets/Tileset_Slope.png");

	for (auto& tex : _textures) SetTextureFilter(tex, TEXTURE_FILTER_POINT); // Important for pixel art
}

void AssetManager::UnloadTextures()
{
	for (auto& tex : _textures) UnloadTexture(tex);
}

const Texture2D& AssetManager::GetTexture(TextureId id) const 
{
	return _textures[(size_t)id];
}