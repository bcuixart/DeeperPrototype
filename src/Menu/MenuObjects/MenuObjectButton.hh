#ifndef MENUOBJECTBUTTON_HH
#define MENUOBJECTBUTTON_HH

#include <raylib.h>

#include "Menu/MenuObject.hh"

class MenuObjectButton : public MenuObject {
public:
	MenuObjectButton(const Rectangle& bounds, Callback onInteractCallback, const std::string& text);
	~MenuObjectButton();

	virtual void Update(const float deltaTime) override;
	virtual void Render(const float deltaTime) override;

	bool IsCursorHovering(const float x, const float y, const int cursorIndex) const override;
	void OnHover(const int cursorIndex) override;
	void OnInteract(const int cursorIndex) override;

private:
	Rectangle _bounds;
	Callback _onInteractCallback;

	std::string _text;

	bool _isHovered;

};

#endif