#include "MenuManager.hh"
#include "GameManager.hh"

MenuManager::MenuManager()
{
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		_menuCursors.push_back(std::make_unique<MenuCursor>(i, false));
	}
}

MenuManager::~MenuManager()
{
}

void MenuManager::Update(const float deltaTime)
{
	for (auto& cursor : _menuCursors) cursor->Update(deltaTime);
	for (auto& object : _menuObjects) object->Update(deltaTime);
}

void MenuManager::Render(const float deltaTime)
{
	BeginDrawing();

	ClearBackground(BLACK);

	for (auto& cursor : _menuCursors) cursor->Render(deltaTime);
	for (auto& object : _menuObjects) object->Render(deltaTime);

	EndDrawing();
}

void MenuManager::LoadMenu(const std::string& menuName)
{

}


void MenuManager::ReceiveInput(const std::array<uint8_t, MAX_LOCAL_PLAYERS>& input)
{
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		_menuCursors[i]->ReceiveInput(input[i]);
	}
}

void MenuManager::SetMenuCursorActive(uint8_t playerIndex, bool isActive)
{
	if (playerIndex >= MAX_LOCAL_PLAYERS) return;
	_menuCursors[playerIndex]->SetActive(isActive);
}