#ifndef LEVELOBJECTSLIST_HH
#define LEVELOBJECTSLIST_HH

#include <cstdint>
#include <unordered_map>

#include "LevelObject.hh"
#include "LevelObjects/LevelObjectDog.hh"
#include "LevelObjects/Tiles/Autotile.hh"
#include "LevelObjects/Tiles/LevelObjectTile.hh"
#include "LevelObjects/Tiles/LevelObjectTileGround.hh"
#include "LevelObjects/Tiles/LevelObjectTileSand.hh"
#include "LevelObjects/Tiles/LevelObjectTileDirt.hh"
#include "LevelObjects/Tiles/LevelObjectTileSlope.hh"
#include "LevelObjects/Tiles/LevelObjectTileBridge.hh"
#include "LevelObjects/Objects/LevelObjectBallTennis.hh"
#include "LevelObjects/Objects/LevelObjectHydrant001.hh"
#include "LevelObjects/Deco/LevelObjectBench001.hh"
#include "LevelObjects/Objects/LevelObjectDogHouse.hh"
#include "LevelObjects/Objects/LevelObjectUmbrellaTall.hh"

enum class LevelObjectCategory : uint8_t { Tile, Interactable, Mobile, Decoration };

constexpr uint8_t kIdGround = 'G';
constexpr uint8_t kIdSlope = 's';
constexpr uint8_t kIdSand = 'S';
constexpr uint8_t kIdDirt = 'd';
constexpr uint8_t kIdBridge = 'B';
constexpr uint8_t kIdTennisBall = 't';
constexpr uint8_t kIdHydrant = 'h';
constexpr uint8_t kIdBench = 'b';
constexpr uint8_t kIdDogHouse = 'D';
constexpr uint8_t kIdUmbrellaTall = 'U';

struct LevelObjectProperties
{
	LevelObjectCategory category;
	Rectangle editorBounds;
	bool canBePlacedByDragging;
};

const std::unordered_map<uint8_t, LevelObjectProperties> levelObjectsList = 
{
	// ID				Category							EditorBounds	CanBePlacedByDragging
	{kIdGround,			{LevelObjectCategory::Tile,			{0, 0, 16, 16}, true}},	 // Ground
	{kIdSlope,			{LevelObjectCategory::Tile,			{0, 0, 16, 16}, true}},	 // Slope
	{kIdSand,			{LevelObjectCategory::Tile,			{0, 0, 16, 16}, true}},	 // Sand
	{kIdDirt,			{LevelObjectCategory::Tile,			{0, 0, 16, 16}, true}},	 // Dirt
	{kIdBridge,			{LevelObjectCategory::Tile,			{0, 0, 16, 16}, true}},	 // Bridge
	{kIdTennisBall,		{LevelObjectCategory::Mobile,		{0, 0, 16, 16}, false}}, // Tennis Ball
	{kIdHydrant,		{LevelObjectCategory::Interactable, {0, 0, 16, 16}, false}}, // Hydrant
	{kIdBench,			{LevelObjectCategory::Decoration,	{0, 0, 32, 16}, false}}, // Bench
	{kIdDogHouse,		{LevelObjectCategory::Interactable, {0, 0, 16, 16}, false}}, // Dog House
	{kIdUmbrellaTall,	{LevelObjectCategory::Decoration,	{0, 0, 16, 32}, false}}	 // Umbrella Tall

};

#endif