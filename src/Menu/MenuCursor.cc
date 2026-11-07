#include "MenuCursor.hh"

MenuCursor::MenuCursor(int id, bool isActive)
{
	ResetCursor();

	_id = id;
	_isActive = isActive;
	_source = CursorSource::Local;
	_localIndex = (id >= 0 && id < MAX_LOCAL_PLAYERS) ? id : -1;
}

MenuCursor::~MenuCursor()
{
}

static float DecodeMenuAxis(uint16_t raw)
{
    const int v = static_cast<int>(raw) - 32;
    return v >= 0 ? v / static_cast<float>(63 - 32)
                  : v / static_cast<float>(32);
}

void MenuCursor::ResetCursor()
{
	_bounds = { 0, 0, 16, 16 };

	_input = 0;
	_prevInput = 0;

	_isActive = false;
	_selecting = false;
	_choosingPoseTimer = 0.0f;
	_pose = CursorPose::Pointer;
	_poseChoice = CursorPose::Pointer;
	_hasRemoteTarget = false;
}

void MenuCursor::Configure(int id, CursorSource source, int localIndex, bool active)
{
	_id = id; _source = source; _localIndex = localIndex; _isActive = active; _hasRemoteTarget = false;
}

void MenuCursor::Update(const float deltaTime)
{
	if (!_isActive) return;

	if (_source == CursorSource::Remote)
	{
		if (!_hasRemoteTarget) return;

		const float k = 1.0f - expf(-deltaTime * kRemoteSmoothing);
		_bounds.x += (_targetX - _bounds.x) * k;
		_bounds.y += (_targetY - _bounds.y) * k;
		_selecting = false;

		return;
	}

	uint16_t axisX = (_input & BITMASK_MENU_AXIS_X) >> 2;
	uint16_t axisY = (_input & BITMASK_MENU_AXIS_Y) >> 10;

	float movementX = DecodeMenuAxis(axisX);
	float movementY = DecodeMenuAxis(axisY);
	const float lenSq = movementX * movementX + movementY * movementY;
	if (lenSq > 1.0f) { const float inv = 1.0f / sqrtf(lenSq); movementX *= inv; movementY *= inv; }

	if (_input & BITMASK_MENU_SELECT || _input & BITMASK_MENU_SECONDARY) _choosingPoseTimer += deltaTime;
	else _choosingPoseTimer = 0.0f;

	if (_choosingPoseTimer >= kTimeToSelectPose)
	{
		float angle = atan2f(-movementY, movementX);
		float magnitude = sqrtf(movementX * movementX + movementY * movementY);

		if (magnitude > 0.5f)
		{
			// 8 directions
			static constexpr CursorPose kPoses[8] = { CursorPose::PointE, CursorPose::PointNE, CursorPose::PointN, CursorPose::PointNW, CursorPose::PointW, CursorPose::PointSW, CursorPose::PointS, CursorPose::PointSE };
			const int sector = (static_cast<int>(roundf(angle / (PI * 0.25f))) + 8) % 8;
			_poseChoice = kPoses[sector];
		}

		SetPose(_poseChoice);

		if (!(_input & BITMASK_MENU_SELECT) && !(_input & BITMASK_MENU_SECONDARY))
		{
			_pose = _poseChoice;
			_choosingPoseTimer = 0.0f;

			SetPose(_poseChoice);
		}

		_selecting = false;
		_prevInput = _input;
		return;
	}	

	_bounds.x += movementX * kCursorSpeed * deltaTime;
	_bounds.y += movementY * kCursorSpeed * deltaTime;

	_bounds.x = std::clamp(_bounds.x, 0.0f, kMenuWidth  - _bounds.width);
	_bounds.y = std::clamp(_bounds.y, 0.0f, kMenuHeight - _bounds.height);

	_selecting = (_input & BITMASK_MENU_SELECT) == 0 && (_prevInput & BITMASK_MENU_SELECT) != 0;

	_prevInput = _input;
}

void MenuCursor::Render(const float deltaTime)
{
	if (!_isActive) return;

	const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::MenuCursorTest);

    Rectangle src = { 16.0f * (float)_pose, 0.0f, 16.0f, 16.0f };
    Rectangle dst = {
        _bounds.x,
        _bounds.y,
        16.0f,
        16.0f
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);

	DrawText(std::to_string(_id + 1).c_str(), _bounds.x + 14, _bounds.y + 10, 8, WHITE);
}

void MenuCursor::ReceiveInput(uint16_t input)
{
	_input = input;
}

void MenuCursor::SetRemoteTarget(float x, float y, CursorPose pose)
{
	_targetX = x;
	_targetY = y;

	SetPose(pose);

	if (!_hasRemoteTarget)
	{
		_bounds.x = x;
		_bounds.y = y;
		_hasRemoteTarget = true;
	}
}