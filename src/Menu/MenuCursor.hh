#ifndef MENUCURSOR_HH
#define MENUCURSOR_HH

#include <raylib.h>

class MenuCursor {
public:
	MenuCursor(int playerIndex, bool isActive);
	~MenuCursor();

	void Update(const float deltaTime);
	void Render(const float deltaTime);

private:
	bool _isActive;
	int _playerIndex;

	Rectangle _bounds;
};

#endif