#ifndef LEVELOBJECT_HH
#define LEVELOBJECT_HH

#include <raylib.h>
#include <raymath.h>
#include <iostream>

class LevelObject {
public:
	LevelObject(const Vector2& position);
	virtual ~LevelObject();

	virtual void Update(const float deltaTime);
	virtual void Render(const float deltaTime) const;

	Vector2 Position() const;

protected:
	Vector2 _position;

private:

};

#endif