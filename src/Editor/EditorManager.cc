#include "EditorManager.hh"

EditorManager::EditorManager() :
    _level(nullptr)
{

}

EditorManager::~EditorManager()
{

}

void EditorManager::Update(const float deltaTime)
{
    // We do not update the level in the editor, as it is not running the game logic. The level is only rendered for editing purposes.
    // _level.Update(deltaTime);

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) _cam.target.x -= kEditorCamMoveSpeed * deltaTime;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) _cam.target.x += kEditorCamMoveSpeed * deltaTime;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) _cam.target.y -= kEditorCamMoveSpeed * deltaTime;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) _cam.target.y += kEditorCamMoveSpeed * deltaTime;

    Vector2 mouseWheel = GetMouseWheelMoveV();
    _cam.zoom += mouseWheel.y * kEditorCamZoomSpeed * deltaTime;

    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), _cam);
}

void EditorManager::Render(const float deltaTime) const
{
    BeginDrawing();
    BeginMode2D(_cam);

    ClearBackground({ 0, 0, 0, 255 });
    _level.RenderBackground(deltaTime);
    _level.RenderEditorGrid(deltaTime);
    _level.RenderObjects(deltaTime);

    if (DEBUG_DRAW_EDITOR_BOUNDS) _level.RenderEditorBounds(deltaTime);

    DrawFPS(-500, -500);

    EndMode2D();
    EndDrawing();
}

void EditorManager::LoadLevel(const std::string& levelName)
{
    _level.LoadLevel(levelName);

    float zoomX = (float)GetScreenWidth() / _level.GetLevelWidth();
    float zoomY = (float)GetScreenHeight() / _level.GetLevelHeight();

    _cam.zoom = std::min(zoomX, zoomY);
    _cam.target = { _level.GetLevelWidth() / 2.0f, _level.GetLevelHeight() / 2.0f };
    _cam.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    _cam.rotation = 0.0f;
}