#ifndef LEVELOBJECTTILESAND_HH
#define LEVELOBJECTTILESAND_HH

#include "LevelObjects/Tiles/LevelObjectTile.hh"

class LevelObjectTileSand : public LevelObjectTile {
public:
	LevelObjectTileSand(const Vector2& position);
	~LevelObjectTileSand();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;
	void RenderBounds(const float deltaTime, const Color& color) const override;

	LevelObjectTileType GetTileType() const override;

protected:

private:

};

#endif