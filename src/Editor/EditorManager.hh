#ifndef EDITORMANAGER_HH
#define EDITORMANAGER_HH

#include <iostream>
#include <sstream>

#include "Level/Level.hh"

class EditorManager {
public:
	EditorManager();
	~EditorManager();

	void Update(const float deltaTime);
	void Render(const float deltaTime) const;

	void LoadLevelFromFile(const std::string& fileName);
	void GetLevelDataToPlaytest(std::ostream& stream) const { _level.WriteLevelToStream(stream); };

	bool CanEnterPlaytest() const { return _level.CanEnterPlaytest(); }

protected:

private:
	void HandleAddingObjects();
	void HandleRemovingObjects();
	
	Level _level;

	Camera2D _cam = { 0 };

	Vector2 _mousePosition = { 0, 0 };
	int _mouseX = 0;
	int _mouseY = 0;
	int _mousePrevX = 0;
	int _mousePrevY = 0;

	uint8_t _selectedObjectId = 0;
	int _selectedObjectEditorOrderIndex = 0;

	constexpr static float kEditorCamMoveSpeed = 1000.0f;
	constexpr static float kEditorCamZoomSpeed = 25.0f;
	constexpr static float kEditorCamMinZoom = 20.0f;
	constexpr static float kEditorCamMaxZoom = 80.0f;

	constexpr static bool DEBUG_DRAW_EDITOR_BOUNDS = false;
};

#endif