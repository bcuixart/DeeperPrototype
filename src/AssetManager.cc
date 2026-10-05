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
	_textures[(size_t)TextureId::MenuCursorTest] = LoadTexture("assets/sprites/menu/Menu_Cursor_Test.png");
	_textures[(size_t)TextureId::DogTest_Long] = LoadTexture("assets/sprites/dog/long/Dog_Long_Idle000.png");
	_textures[(size_t)TextureId::DogTest_Hairy] = LoadTexture("assets/sprites/dog/hairy/Dog_Hairy_Idle000.png");
	_textures[(size_t)TextureId::DogTest_Derpy] = LoadTexture("assets/sprites/dog/derpy/Dog_Derpy_Idle000.png");
	_textures[(size_t)TextureId::DogTest_Deeerpy] = LoadTexture("assets/sprites/dog/deeerpy/Dog_Deeerpy_Idle000.png");
	_textures[(size_t)TextureId::DogTest_Puppy] = LoadTexture("assets/sprites/dog/puppy/Dog_Puppy_Idle000.png");
	_textures[(size_t)TextureId::TilesetGround] = LoadTexture("assets/sprites/tilesets/Tileset_Grass.png");
	_textures[(size_t)TextureId::TilesetSand] = LoadTexture("assets/sprites/tilesets/Tileset_Sand.png");
	_textures[(size_t)TextureId::TilesetDirt] = LoadTexture("assets/sprites/tilesets/Tileset_Dirt.png");
	_textures[(size_t)TextureId::TilesetBridge] = LoadTexture("assets/sprites/tilesets/Tileset_Bridge.png");
	_textures[(size_t)TextureId::BallTennis] = LoadTexture("assets/sprites/objects/Object_Ball_Tennis.png");
	_textures[(size_t)TextureId::Hydrant001] = LoadTexture("assets/sprites/objects/Object_Hydrant001.png");
	_textures[(size_t)TextureId::Hydrant001Water] = LoadTexture("assets/sprites/objects/Object_Hydrant001_Water.png");
	_textures[(size_t)TextureId::Bench001] = LoadTexture("assets/sprites/deco/Deco_Bench_001.png");
	_textures[(size_t)TextureId::DogHouse] = LoadTexture("assets/sprites/objects/Object_DogHouse.png");
	_textures[(size_t)TextureId::UmbrellaTall] = LoadTexture("assets/sprites/objects/Object_Umbrella_Tall.png");

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