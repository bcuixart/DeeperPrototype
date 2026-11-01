#ifndef EDITORMANAGER_HH
#define EDITORMANAGER_HH

#include "Level/Level.hh"

class EditorManager {
public:
	EditorManager();
	~EditorManager();

	void Update(const float deltaTime);
	void Render(const float deltaTime) const;

	void LoadLevel(const std::string& levelName);

protected:

private:
	Level _level;

	Camera2D _cam = { 0 };

	constexpr static float kEditorCamMoveSpeed = 1000.0f;
	constexpr static float kEditorCamZoomSpeed = 25.0f;
	constexpr static float kEditorCamMinZoom = 20.0f;
	constexpr static float kEditorCamMaxZoom = 80.0f;

	constexpr static bool DEBUG_DRAW_EDITOR_BOUNDS = true;
};

#endif