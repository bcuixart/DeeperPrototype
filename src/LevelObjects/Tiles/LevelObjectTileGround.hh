#ifndef LEVELOBJECTTILEGROUND_HH
#define LEVELOBJECTTILEGROUND_HH

#include "LevelObjects/Tiles/LevelObjectTile.hh"

class LevelObjectTileGround : public LevelObjectTile {
public:
	LevelObjectTileGround(const Vector2& position);
	~LevelObjectTileGround();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;
	void RenderBounds(const float deltaTime, const Color& color) const override;

	LevelObjectTileType GetTileType() const override;

protected:

private:

};

#endif