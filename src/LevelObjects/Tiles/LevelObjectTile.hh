#ifndef LEVELOBJECTTILE_HH
#define LEVELOBJECTTILE_HH

#include <cstdint>

#include "LevelObject.hh"

enum class LevelObjectTileType { None, Ground, Slope, Sand, Dirt, Bridge };

class LevelObjectTile : public LevelObject {
public:
	LevelObjectTile(const Vector2& position);
	~LevelObjectTile();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;

	HitboxType GetHitboxType() const override { return HitboxType::Solid; }

	virtual LevelObjectTileType GetTileType() const;

	virtual void SetSpriteIndex(uint8_t idx);
	uint8_t GetSpriteIndex() const;

protected:
	Vector2 _position{};

	int _groundTileType = 0;

	uint8_t _spriteIndex = 0;

private:

};

#endif