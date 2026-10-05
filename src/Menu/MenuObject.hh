#ifndef MENUOBJECT_HH
#define MENUOBJECT_HH

#include <raylib.h>
#include <iostream>
#include <functional>

class MenuObject {
public:
	using Callback = std::function<void(int cursorIndex)>;

	MenuObject();
	~MenuObject();

	virtual void Update(const float deltaTime);
	virtual void Render(const float deltaTime);

	virtual bool IsCursorHovering(const float x, const float y, const int cursorIndex) const = 0;
	virtual void OnHover(const int cursorIndex) {};
	virtual void OnInteract(const int cursorIndex) {};

private:

};

#endif