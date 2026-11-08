#ifndef LEVELOBJECTSLIST_HH
#define LEVELOBJECTSLIST_HH

#include <cstdint>
#include <unordered_map>
#include <vector>

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
#include "LevelObjects/Objects/LevelObjectBone.hh"

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
constexpr uint8_t kBone = 'o';

const std::vector<uint8_t> kLevelObjectEditorOrder = { kIdGround, kIdSlope, kIdSand, kIdDirt, kIdBridge, kIdTennisBall, kIdHydrant, kIdBench, kIdDogHouse, kIdUmbrellaTall, kBone };

struct LevelObjectProperties
{
	LevelObjectCategory category;
	Rectangle editorBounds;
	bool canBePlacedByDragging;
};

const std::unordered_map<uint8_t, LevelObjectProperties> levelObjectsList = 
{
	// ID				Category								EditorBounds					CanBePlacedByDragging
	{kIdGround,			{LevelObjectCategory::Tile,				{0, 0, 1, 1},					true}},					// Ground
	{kIdSlope,			{LevelObjectCategory::Tile,				{0, 0, 1, 1},					true}},					// Slope
	{kIdSand,			{LevelObjectCategory::Tile,				{0, 0, 1, 1},					true}},					// Sand
	{kIdDirt,			{LevelObjectCategory::Tile,				{0, 0, 1, 1},					true}},					// Dirt
	{kIdBridge,			{LevelObjectCategory::Tile,				{0, 0, 1, 1},					true}},					// Bridge
	{kIdTennisBall,		{LevelObjectCategory::Mobile,			{0.25f, 0.25f, 0.5f, 0.5f},		false}},				// Tennis Ball
	{kIdHydrant,		{LevelObjectCategory::Interactable,		{0, -0.5f, 1, 1.5f},			false}},				// Hydrant
	{kIdBench,			{LevelObjectCategory::Decoration,		{-0.25f,-0.1875f, 1.5f,1.1875f},false}},				// Bench
	{kIdDogHouse,		{LevelObjectCategory::Interactable,		{0, -0.5f, 1, 1.5f},			false}},				// Dog House
	{kIdUmbrellaTall,	{LevelObjectCategory::Decoration,		{-0.5f, -2.0f, 2.0f, 3},		false}},				// Umbrella Tall
	{kBone,				{LevelObjectCategory::Interactable,		{0.0625f, 0.25f, 0.875f, 0.4375f},false}}				// Bone

};

#include "LevelBackgrounds/LevelBackground.hh"
#include "LevelBackgrounds/LevelBackgroundPark.hh"
#include "LevelBackgrounds/LevelBackgroundHood.hh"
#include "LevelBackgrounds/LevelBackgroundCity.hh"
#include "LevelBackgrounds/LevelBackgroundSnow.hh"
#include "LevelBackgrounds/LevelBackgroundBeach.hh"

enum class LevelBackgroundType : uint8_t { Park, Hood, City, Snow, Beach, Count };

#endif