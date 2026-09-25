#ifndef LEVELOBJECTTILEBRIDGE_HH
#define LEVELOBJECTTILEBRIDGE_HH

#include "LevelObjects/Tiles/LevelObjectTile.hh"

class LevelObjectTileBridge : public LevelObjectTile {
public:
	LevelObjectTileBridge(const Vector2& position);
	~LevelObjectTileBridge();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;

	HitboxType GetHitboxType() const override { return HitboxType::SemisolidTotal; }

	LevelObjectTileType GetTileType() const override;

	void OnDug(LevelObject* digger) override;

protected:

private:

};

#endif