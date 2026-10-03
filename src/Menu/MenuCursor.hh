#ifndef MENUCURSOR_HH
#define MENUCURSOR_HH

class MenuCursor {
public:
	MenuCursor(int playerIndex, bool isActive);
	~MenuCursor();

	void Update(const float deltaTime);
	void Render(const float deltaTime);

private:
	bool _isActive;
	int _playerIndex;
};

#endif