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
}

void MenuManager::Render(const float deltaTime)
{
	BeginDrawing();

	EndDrawing();
}

void MenuManager::LoadMenu(const std::string& menuName)
{

}