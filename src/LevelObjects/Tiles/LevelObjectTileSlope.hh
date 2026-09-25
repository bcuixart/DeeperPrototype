#ifndef LEVELOBJECTTILESLOPE_HH
#define LEVELOBJECTTILESLOPE_HH

#include "LevelObjects/Tiles/LevelObjectTile.hh"

class LevelObjectTileSlope : public LevelObjectTile {
public:
	LevelObjectTileSlope(const Vector2& position);
	~LevelObjectTileSlope();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;
	void RenderBounds(const float deltaTime, const Color& color) const override;

	Vector2 SampleLeftRightAtY(const float y) const override;
	Vector2 SampleTopBottomAtX(const float x) const override;

	HitboxType GetHitboxType() const override { return HitboxType::SolidMaybeSloped; }

	bool HasActualSlopedHitbox() const override { return _isSlopedTile; }

	LevelObjectTileType GetTileType() const override;

	virtual void SetSpriteIndex(uint8_t idx) override;

	void OnDug(LevelObject* digger) override;

protected:

private:
	// Initialized as true because autotile needs to assume it is a slope,
	// and then it will be set to false if the sprite index is not a slope.
	bool _isSlopedTile{true};

	SlopeOrientation _slopeOrientation{SlopeOrientation::None};
};

#endif