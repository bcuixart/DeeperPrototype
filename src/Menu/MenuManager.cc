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

	for (uint8_t t = 0; t < kNumDogTypes; ++t)
	{
		_menuObjectsCallbacks["lobby_dog_" + std::to_string(t)] = [this, t](int cursorIndex)
		{
			_pendingMenuObjectInteractions.push_back([cursorIndex, t] { GameManager::instance->OnLobbyPickDogType(cursorIndex, t); });
		};
	}
	_menuObjectsCallbacks["lobby_start"] = [this](int cursorIndex)
	{
		_pendingMenuObjectInteractions.push_back([cursorIndex] { GameManager::instance->OnLobbyStartPressed(cursorIndex); });
	};

	LoadMenu("join");
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

void MenuManager::Render(const float deltaTime, const std::function<void()>& overlay)
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

	if (overlay) overlay();
	for (auto& object : _menuObjects) object->Render(deltaTime);
	for (auto& cursor : _menuCursors) cursor->Render(deltaTime);

	EndMode2D();

	EndDrawing();
}

void MenuManager::LoadMenu(const std::string& menuName)
{
	_menuObjects.clear();

	if (menuName == "join")
	{
		_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 78, 95, 100, 15 }, _menuObjectsCallbacks.at("start_game"), "Start game"));
		_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 78, 120, 100, 15 }, _menuObjectsCallbacks.at("remove_player"), "Self-destruct player"));

		_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 50, 15, 50, 15 }, _menuObjectsCallbacks.at("set_dog_type_long"), "Long dog"));
		_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 156, 15, 50, 15 }, _menuObjectsCallbacks.at("set_dog_type_hairy"), "Hairy dog"));
		_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 50, 40, 50, 15 }, _menuObjectsCallbacks.at("set_dog_type_derpy"), "Derpy dog"));
		_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 156, 40, 50, 15 }, _menuObjectsCallbacks.at("set_dog_type_deeerpy"), "Deeeeeerpy dog"));
		_menuObjects.push_back(std::make_unique<MenuObjectButton>(Rectangle{ 103, 60, 50, 15 }, _menuObjectsCallbacks.at("set_dog_type_puppy"), "Puppy"));
	}
	else if (menuName == "lobby")
	{
		const float W = kMenuWidth, H = kMenuHeight;

		if (kAllowLobbyDogChange)
		{
			static const char* kDogNames[kNumDogTypes] = { "Long", "Hairy", "Derpy", "Deeerpy", "Puppy" };
			for (int t = 0; t < kNumDogTypes; ++t)
			{
				_menuObjects.push_back(std::make_unique<MenuObjectButton>(
					Rectangle{ W * 0.58f + (t % 2) * W * 0.11f, H * 0.17f + (t / 2) * H * 0.11f, W * 0.10f, H * 0.085f },
					_menuObjectsCallbacks.at("lobby_dog_" + std::to_string(t)), kDogNames[t]));
			}
		}

		_menuObjects.push_back(std::make_unique<MenuObjectButton>(
			Rectangle{ W * 0.58f, H * 0.58f, W * 0.22f, H * 0.085f },
			_menuObjectsCallbacks.at("lobby_start"), "Comencar partida"));
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

void MenuManager::ReceiveInput(const std::array<uint8_t, MAX_LOCAL_PLAYERS>& input)
{
	for (auto& cursor : _menuCursors)
	{
		const int li = cursor->GetLocalIndex();
		if (cursor->IsLocal() && li >= 0 && li < MAX_LOCAL_PLAYERS) cursor->ReceiveInput(input[li]);
	}
}

void MenuManager::ConfigureLocalCursors(uint8_t activeCount)
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		auto& c = *_menuCursors[i];
		c.ResetCursor();
		c.Configure(i, CursorSource::Local, i < MAX_LOCAL_PLAYERS ? i : -1, i < activeCount);
	}
}

void MenuManager::ConfigureOnlineCursors(const LobbyState& lobby, uint8_t myMachineId)
{
	int nextLocal = 0;
	for (int s = 0; s < MAX_PLAYERS; ++s)
	{
		auto& c = *_menuCursors[s];
		const SlotInfo& slot = lobby.slots[s];

		if (!slot.Occupied()) { c.ResetCursor(); continue; }   // ResetCursor el deixa inactiu

		const bool mine = slot.machineId == myMachineId;
		const CursorSource source = mine ? CursorSource::Local : CursorSource::Remote;
		const int localIndex = mine ? nextLocal++ : -1;

		// Si ja estava així, no el toquem: no ha de saltar de lloc quan algú altre entra o surt.
		if (c.IsActive() && c.GetPlayerIndex() == s && c.GetSource() == source && c.GetLocalIndex() == localIndex) continue;

		c.ResetCursor();
		c.Configure(s, source, localIndex, true);
		c.SetPosition(kMenuWidth * 0.5f + s * 24.0f, kMenuHeight * 0.5f);
	}
}

static uint16_t ToWire(float v)
{
	return (uint16_t)(std::clamp(v, 0.0f, 4095.0f) * kCursorScale);
}

void MenuManager::CollectLocalCursors(std::vector<CursorState>& out) const
{
	for (const auto& c : _menuCursors)
	{
		if (!c->IsActive() || !c->IsLocal()) continue;
		out.push_back(CursorState{ (uint8_t)c->GetPlayerIndex(), ToWire(c->GetX()), ToWire(c->GetY()), (uint8_t)c->GetPose() });
	}
}

void MenuManager::ApplyRemoteCursor(const CursorState& state)
{
	if (state.slot >= _menuCursors.size()) return;
	auto& c = *_menuCursors[state.slot];
	if (!c.IsActive() || c.IsLocal()) return;   // mai deixem que la xarxa mogui un cursor nostre
	c.SetRemoteTarget(state.x / kCursorScale, state.y / kCursorScale, (CursorPose)state.pose);
}