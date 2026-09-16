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
	bool _isSlopedTile{false};
};

#endif