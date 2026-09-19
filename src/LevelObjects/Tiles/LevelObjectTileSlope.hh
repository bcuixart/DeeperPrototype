#ifndef LEVELOBJECTTILESLOPE_HH
#define LEVELOBJECTTILESLOPE_HH

#include "LevelObjects/Tiles/LevelObjectTile.hh"

class LevelObjectTileSlope : public LevelObjectTile {
public:
	LevelObjectTileSlope(const Vector2& position);
	~LevelObjectTileSlope();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;

	LevelObjectTileType GetTileType() const override;

	virtual void SetSpriteIndex(uint8_t idx) override;

protected:

private:
	// Initialized as true because autotile needs to assume it is a slope for the first pass,
	// and then it will be set to false if the sprite index is not a slope.
	bool _isSlopedTile{true};
};

#endif