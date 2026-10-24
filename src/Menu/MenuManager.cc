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

	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 78, 95, 100, 15 }, _menuObjectsCallbacks.at("start_game"), "Start game"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 78, 120, 100, 15 }, _menuObjectsCallbacks.at("remove_player"), "Self-destruct player"));

	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 50, 15, 50, 15 }, _menuObjectsCallbacks.at("set_dog_type_long"), "Long dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 156, 15, 50, 15 }, _menuObjectsCallbacks.at("set_dog_type_hairy"), "Hairy dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 50, 40, 50, 15 }, _menuObjectsCallbacks.at("set_dog_type_derpy"), "Derpy dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 156, 40, 50, 15 }, _menuObjectsCallbacks.at("set_dog_type_deeerpy"), "Deeeeeerpy dog"));
	_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 103, 60, 50, 15 }, _menuObjectsCallbacks.at("set_dog_type_puppy"), "Puppy"));
}

MenuManager::~MenuManager()
{
}


void MenuManager::Update(const float deltaTime)
{
	for (auto& cursor : _menuCursors) cursor->Update(deltaTime);
	for (auto& object : _menuObjects) object->Update(deltaTime);

	for (auto& cursor : _menuCursors)
	{
		if (!cursor->IsActive()) continue;
		for (auto& object : _menuObjects)
		{
			if (!object->IsCursorHovering(cursor->GetX(), cursor->GetY(), cursor->GetPlayerIndex())) continue;

			object->OnHover(cursor->GetPlayerIndex());

			if (cursor->IsLocal() && cursor->IsSelecting()) object->OnInteract(cursor->GetPlayerIndex());
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

	const float sw = (float)GetScreenWidth();
	const float sh = (float)GetScreenHeight();
	const float zoom = std::min(sw / kMenuWidth, sh / kMenuHeight);

	Camera2D cam{};
	cam.zoom = zoom;
	cam.target = { 0.0f, 0.0f };
	cam.offset = { (sw - kMenuWidth * zoom) * 0.5f, (sh - kMenuHeight * zoom) * 0.5f };

	BeginMode2D(cam);

	DrawRectangle(0, 0, (int)kMenuWidth, (int)kMenuHeight, { 20, 20, 28, 255 });
	DrawRectangleLines(0, 0, (int)kMenuWidth, (int)kMenuHeight, { 70, 70, 90, 255 });

	for (auto& cursor : _menuCursors) cursor->Render(deltaTime);
	for (auto& object : _menuObjects) object->Render(deltaTime);

	EndMode2D();

	EndDrawing();
}

void MenuManager::LoadMenu(const std::string& menuName)
{

}


void MenuManager::ReceiveInput(const std::array<uint8_t, MAX_LOCAL_PLAYERS>& input)
{
	for (auto& cursor : _menuCursors)
	{
		const int li = cursor->GetLocalIndex();
		if (cursor->IsLocal() && li >= 0 && li < MAX_LOCAL_PLAYERS) cursor->ReceiveInput(input[li]);
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

void MenuManager::SetCursorPose(uint8_t id, CursorPose pose)
{
	if (id >= _menuCursors.size()) return;
	_menuCursors[id]->SetPose(pose);
}