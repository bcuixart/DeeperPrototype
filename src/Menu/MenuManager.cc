#include "MenuManager.hh"
#include "GameManager.hh"

MenuManager::MenuManager()
{
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		_menuCursors.push_back(std::make_unique<MenuCursor>(i, false));
	}

    _menuObjectsCallbacks = {
        { "start_game",    [this](int)             { _pendingMenuObjectInteractions.push_back([]                 { GameManager::instance->OnPlayerSelectedStartGame(); }); } },
        { "remove_player", [this](int cursorIndex) { _pendingMenuObjectInteractions.push_back([cursorIndex]      { GameManager::instance->TryToRemovePlayer(cursorIndex); }); } },
        { "set_dog_type_long",  [this](int cursorIndex)  { _pendingMenuObjectInteractions.push_back([cursorIndex]       { GameManager::instance->SetPlayerDogType(cursorIndex, 0); }); } },
        { "set_dog_type_hairy",  [this](int cursorIndex)  { _pendingMenuObjectInteractions.push_back([cursorIndex]       { GameManager::instance->SetPlayerDogType(cursorIndex, 1); }); } },
        { "set_dog_type_derpy",  [this](int cursorIndex)  { _pendingMenuObjectInteractions.push_back([cursorIndex]       { GameManager::instance->SetPlayerDogType(cursorIndex, 2); }); } },
        { "set_dog_type_deeerpy",  [this](int cursorIndex)  { _pendingMenuObjectInteractions.push_back([cursorIndex]       { GameManager::instance->SetPlayerDogType(cursorIndex, 3); }); } },
        { "set_dog_type_puppy",  [this](int cursorIndex)  { _pendingMenuObjectInteractions.push_back([cursorIndex]       { GameManager::instance->SetPlayerDogType(cursorIndex, 4); }); } },
    };

	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 100, 350, 200, 50 }, _menuObjectsCallbacks.at("start_game"), "Start game"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 100, 425, 200, 50 }, _menuObjectsCallbacks.at("remove_player"), "Self-destruct player"));

	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 100, 100, 50, 50 }, _menuObjectsCallbacks.at("set_dog_type_long"), "Long dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 250, 100, 50, 50 }, _menuObjectsCallbacks.at("set_dog_type_hairy"), "Hairy dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 100, 175, 50, 50 }, _menuObjectsCallbacks.at("set_dog_type_derpy"), "Derpy dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 250, 175, 50, 50 }, _menuObjectsCallbacks.at("set_dog_type_deeerpy"), "Deeeeeerpy dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 166, 250, 50, 50 }, _menuObjectsCallbacks.at("set_dog_type_puppy"), "Puppy"));
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

    auto pending = std::move(_pendingMenuObjectInteractions);
    _pendingMenuObjectInteractions.clear();
    for (auto& action : pending) action();
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

void MenuManager::RemoveCursorAndShift(uint8_t removedIndex, uint8_t activeCountBefore)
{
    if (removedIndex >= activeCountBefore || activeCountBefore > MAX_LOCAL_PLAYERS) return;

    auto first = _menuCursors.begin() + removedIndex;
    auto last  = _menuCursors.begin() + activeCountBefore;
    std::rotate(first, first + 1, last);

    for (int i = removedIndex; i < activeCountBefore; i++)
        _menuCursors[i]->SetPlayerIndex(i);

    _menuCursors[activeCountBefore - 1]->ResetCursor();
}