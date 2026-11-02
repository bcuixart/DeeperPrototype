#ifndef LEVELBACKGROUND_HH
#define LEVELBACKGROUND_HH

#include <raylib.h>

#include "AssetManager.hh"

class LevelBackground {
public:
	LevelBackground(const int width, const int height) : _width(width), _height(height) {};
	~LevelBackground() {};

	virtual void Update(const float deltaTime) = 0;
	virtual void Render(const float deltaTime) const = 0;

protected:
	const int _width = 0;
	const int _height = 0;

private:

};

#endif