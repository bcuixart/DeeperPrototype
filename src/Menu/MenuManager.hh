#ifndef MENUMANAGER_HH
#define MENUMANAGER_HH

#include <vector>
#include <array>
#include <functional>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <iostream>
#include <raylib.h>

#include "InputManager.hh"
#include "MenuCursor.hh"
#include "MenuObject.hh"
#include "MenuObjects/MenuObjectButton.hh"

class GameManager;

class MenuManager {
public:
	MenuManager();
	~MenuManager();

	void Update(const float deltaTime);
	void Render(const float deltaTime);

	void LoadMenu(const std::string& menuName);

	void ReceiveInput(const std::array<uint8_t, MAX_LOCAL_PLAYERS>& input);

	void SetMenuCursorActive(uint8_t playerIndex, bool isActive);
	void RemoveCursorAndShift(uint8_t removedIndex, uint8_t activeCountBefore);

private:
	std::vector<std::unique_ptr<MenuCursor>> _menuCursors;
	std::vector<std::unique_ptr<MenuObject>> _menuObjects;

    std::unordered_map<std::string, MenuObject::Callback> _menuObjectsCallbacks;
    std::vector<std::function<void()>> _pendingMenuObjectInteractions;
};

#endif