#ifndef MENUCURSOR_HH
#define MENUCURSOR_HH

#include <raylib.h>
#include <cstdint>
#include <string>
#include <algorithm>

#include "InputManager.hh"
#include "AssetManager.hh"
#include "MenuConstants.hh"

enum class CursorPose : uint8_t { Pointer = 0, Rock, Paper, Scissors, ThumbsUp, ThumbsDown, TBA1, TBA2, PointN, PointNE, PointE, PointSE, PointS, PointSW, PointW, PointNW, Count };

enum class CursorSource : uint8_t { Local, Remote };

class MenuCursor {
public:
	MenuCursor(int id, bool isActive);
	~MenuCursor();

	void ResetCursor();
	void Configure(int id, CursorSource source, int localIndex, bool active);

	void Update(const float deltaTime);
	void Render(const float deltaTime);

	void ReceiveInput(uint8_t input);
	bool IsSelecting() const { return _selecting; }

	void SetRemoteTarget(float x, float y, CursorPose pose);
	void SetSource(CursorSource source, int localIndex = -1) { _source = source; _localIndex = localIndex; }
	CursorSource GetSource() const { return _source; }
	bool IsLocal() const { return _source == CursorSource::Local; }
	int GetLocalIndex() const { return _localIndex; }

	void SetActive(bool isActive) { _isActive = isActive; }
	bool IsActive() const { return _isActive; }

	float GetX() const { return _bounds.x; }
	float GetY() const { return _bounds.y; }
	void SetPosition(float x, float y) { _bounds.x = x; _bounds.y = y; }

	CursorPose GetPose() const { return _pose; }
	void SetPose(CursorPose pose) { if (pose < CursorPose::Count) _pose = pose; }

	int GetPlayerIndex() const { return _id; }
	void SetPlayerIndex(int index) { _id = index; if (_source == CursorSource::Local && _localIndex >= 0) _localIndex = index; }

private:
	bool _isActive;
	int _id;

	CursorSource _source;
	int _localIndex;

	Rectangle _bounds;

	uint8_t _input;
	uint8_t _prevInput;

	bool _selecting;

	float _choosingPoseTimer = 0.0f;
	CursorPose _poseChoice;
	CursorPose _pose;

	float _targetX = 0.0f;
	float _targetY = 0.0f;
	bool _hasRemoteTarget = false;

	static constexpr float kCursorSpeed = 100.0f;
	static constexpr float kRemoteSmoothing = 20.0f;

	static constexpr float kTimeToSelectPose = 0.5f;
};

#endif