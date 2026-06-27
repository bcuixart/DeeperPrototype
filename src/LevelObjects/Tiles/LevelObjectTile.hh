#ifndef LEVELOBJECTTILE_HH
#define LEVELOBJECTTILE_HH

#include "LevelObject.hh"

enum class LevelObjectTileType { None, Ground, Slope, Sand, Dirt, OneWayFloor };

class LevelObjectTile : public LevelObject {
public:
	LevelObjectTile(const Vector2& position);
	~LevelObjectTile();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;

	virtual LevelObjectTileType GetTileType() const;

protected:
	int _groundTileType = 0;

private:

};

#endif