#ifndef LEVELOBJECTTILEDIRT_HH
#define LEVELOBJECTTILEDIRT_HH

#include "Level/LevelObjects/Tiles/LevelObjectTile.hh"

class LevelObjectTileDirt : public LevelObjectTile {
public:
	LevelObjectTileDirt(const Vector2& position);
	~LevelObjectTileDirt();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;
	void RenderBounds(const float deltaTime, const Color& color) const override;

	LevelObjectTileType GetTileType() const override;

	void CollidedWithX(LevelObject* other, float prevVelocityX) override;
    void CollidedWithY(LevelObject* other, float prevVelocityY) override;

	void OnDug(LevelObject* digger) override;

protected:

private:

};

#endif