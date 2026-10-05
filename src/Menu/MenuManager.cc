#include "MenuManager.hh"
#include "GameManager.hh"

MenuManager::MenuManager()
{
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		_menuCursors.push_back(std::make_unique<MenuCursor>(i, false));
	}

	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 100, 350, 200, 50 }, "Start game"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 100, 425, 200, 50 }, "Self-destruct player"));

	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 100, 100, 50, 50 }, "Long dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 250, 100, 50, 50 }, "Hairy dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 100, 175, 50, 50 }, "Derpy dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 250, 175, 50, 50 }, "Deeeeeerpy dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 166, 250, 50, 50 }, "Puppy"));
}

MenuManager::~MenuManager()
{
}

void MenuManager::Update(const float deltaTime)
{
	for (auto& cursor : _menuCursors) cursor->Update(deltaTime);
	for (auto& object : _menuObjects) object->Update(deltaTime);

	for(auto& cursor : _menuCursors)
	{
		if (!cursor->IsActive()) continue;
		for (auto& object : _menuObjects)
		{
			if (object->IsCursorHovering(cursor->GetX(), cursor->GetY(), cursor->GetPlayerIndex()))
			{
				object->OnHover(cursor->GetPlayerIndex());

				if (cursor->IsSelecting()) object->OnInteract(cursor->GetPlayerIndex());
			}
		}
	}
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