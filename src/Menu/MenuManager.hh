#ifndef MENUMANAGER_HH
#define MENUMANAGER_HH

#include <vector>
#include <array>
#include <memory>
#include <fstream>
#include <iostream>
#include <raylib.h>

#include "MenuCursor.hh"
#include "MenuObject.hh"

class MenuManager {
public:
	MenuManager();
	~MenuManager();

	void Update(const float deltaTime);
	void Render(const float deltaTime);

	void LoadMenu(const std::string& menuName);

private:
	std::vector<std::unique_ptr<MenuCursor>> _menuCursors;
	std::vector<std::unique_ptr<MenuObject>> _menuObjects;
};

#endif