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

	uint8_t axisX = (_input & BITMASK_MENU_AXIS_X) >> 2;
	uint8_t axisY = (_input & BITMASK_MENU_AXIS_Y) >> 5;

	float movementX = 0.0f;
	if (axisX == 0x00) movementX = -1.0f;
	else if (axisX == 0x01) movementX = -0.5f;
	else if (axisX == 0x02) movementX = -0.25f;
	else if (axisX == 0x05) movementX = 0.25f;
	else if (axisX == 0x06) movementX = 0.5f;
	else if (axisX == 0x07) movementX = 1.0f;

	float movementY = 0.0f;
	if (axisY == 0x00) movementY = -1.0f;
	else if (axisY == 0x01) movementY = -0.5f;
	else if (axisY == 0x02) movementY = -0.25f;
	else if (axisY == 0x05) movementY = 0.25f;
	else if (axisY == 0x06) movementY = 0.5f;
	else if (axisY == 0x07) movementY = 1.0f;

	if (_input & BITMASK_MENU_SELECT || _input & BITMASK_MENU_SECONDARY) _choosingPoseTimer += deltaTime;
	else _choosingPoseTimer = 0.0f;

	if (_choosingPoseTimer >= kTimeToSelectPose)
	{
		float angle = atan2f(-movementY, movementX);
		float magnitude = sqrtf(movementX * movementX + movementY * movementY);

		if (magnitude > 0.5f)
		{
			// 8 directions
			if (angle >= 0 && angle < PI / 8.0f) _poseChoice = CursorPose::PointE;
			else if (angle >= PI / 8.0f && angle < 3.0f * PI / 8.0f) _poseChoice = CursorPose::PointNE;
			else if (angle >= 3.0f * PI / 8.0f && angle < 5.0f * PI / 8.0f) _poseChoice = CursorPose::PointN;
			else if (angle >= 5.0f * PI / 8.0f && angle < 7.0f * PI / 8.0f) _poseChoice = CursorPose::PointNW;
			else if (angle >= 7.0f * PI / 8.0f || angle < -7.0f * PI / 8.0f) _poseChoice = CursorPose::PointW;
			else if (angle >= -7.0f * PI / 8.0f && angle < -5.0f * PI / 8.0f) _poseChoice = CursorPose::PointSW;
			else if (angle >= -5.0f * PI / 8.0f && angle < -3.0f * PI / 8.0f) _poseChoice = CursorPose::PointS;
			else if (angle >= -3.0f * PI / 8.0f && angle < -1.0f * PI / 8.0f) _poseChoice = CursorPose::PointSE;
		}

		SetPose(_poseChoice);

		if (!_input & BITMASK_MENU_SELECT && !_input & BITMASK_MENU_SECONDARY)
		{
			_pose = _poseChoice;
			_choosingPoseTimer = 0.0f;

			SetPose(_poseChoice);
		}

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

void MenuCursor::ReceiveInput(uint8_t input)
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