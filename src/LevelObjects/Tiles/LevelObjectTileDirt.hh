#ifndef LEVELOBJECTTILEDIRT_HH
#define LEVELOBJECTTILEDIRT_HH

#include "LevelObjects/Tiles/LevelObjectTile.hh"

class LevelObjectTileDirt : public LevelObjectTile {
public:
	LevelObjectTileDirt(const Vector2& position);
	~LevelObjectTileDirt();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;
	void RenderBounds(const float deltaTime, const Color& color) const override;

	LevelObjectTileType GetTileType() const override;

protected:

private:

};

#endif