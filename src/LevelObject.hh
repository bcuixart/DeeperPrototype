#ifndef LEVELOBJECT_HH
#define LEVELOBJECT_HH

#include <raylib.h>
#include <raymath.h>
#include <iostream>

#include "AssetManager.hh"

class LevelManager;

class LevelObject {
public:
	LevelObject();
	virtual ~LevelObject();

	virtual void Update(const float deltaTime);
	virtual void Render(const float deltaTime) const;
	virtual void RenderBounds(const float deltaTime, const Color& color) const;

	Rectangle GetBounds() const { return _bounds; }
	Vector2 GetPosition() const { return { _bounds.x, _bounds.y }; }
	Vector2 GetVelocity() const { return _velocity; }

protected:
	Rectangle _bounds{};
	Vector2   _velocity{};

private:

};

#endif