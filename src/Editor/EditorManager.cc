#include "EditorManager.hh"

EditorManager::EditorManager() :
    _level(nullptr)
{
    _selectedObjectId = kLevelObjectEditorOrder[_selectedObjectEditorOrderIndex];
}

EditorManager::~EditorManager()
{

}

void EditorManager::Update(const float deltaTime)
{
    // We do not update the level in the editor, as it is not running the game logic. The level is only rendered for editing purposes.
    // _level.Update(deltaTime);

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) _cam.target.x -= kEditorCamMoveSpeed / _cam.zoom * deltaTime;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) _cam.target.x += kEditorCamMoveSpeed / _cam.zoom * deltaTime;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) _cam.target.y -= kEditorCamMoveSpeed / _cam.zoom * deltaTime;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) _cam.target.y += kEditorCamMoveSpeed / _cam.zoom * deltaTime;

    Vector2 mouseWheel = GetMouseWheelMoveV();
    _cam.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    _cam.zoom = Clamp(_cam.zoom * powf(1.1f, mouseWheel.y), kEditorCamMinZoom, kEditorCamMaxZoom);

    _mousePosition = GetScreenToWorld2D(GetMousePosition(), _cam);
    _mouseX = (int)floorf(_mousePosition.x);
    _mouseY = (int)floorf(_mousePosition.y);    

    if (IsKeyPressed(KEY_Q)) _selectedObjectEditorOrderIndex = (_selectedObjectEditorOrderIndex + 1) % kLevelObjectEditorOrder.size();
    _selectedObjectId = kLevelObjectEditorOrder[_selectedObjectEditorOrderIndex];

    if (IsKeyPressed(KEY_C)) _cam = _level.FitCameraToBounds(1.0f, 16.0f, 9.0f);

    HandleAddingObjects();
    HandleRemovingObjects();

    _mousePrevX = _mouseX;
    _mousePrevY = _mouseY;
}

void EditorManager::Render(const float deltaTime) const
{
    BeginDrawing();
    BeginMode2D(_cam);

    ClearBackground({ 0, 0, 0, 255 });
    _level.RenderBackground(deltaTime);
    _level.RenderEditorGrid(deltaTime);
    _level.RenderObjects(deltaTime);
    _level.RenderEditorDraggingPreview(deltaTime, _selectedObjectId, _mouseX, _mouseY);
    _level.RenderEditorGridBounds(deltaTime);

    if (DEBUG_DRAW_EDITOR_BOUNDS) _level.RenderEditorObjectBounds(deltaTime);

    DrawFPS(-500, -500);

    EndMode2D();
    EndDrawing();
}

void EditorManager::LoadLevel(const std::string& levelName)
{
    _level.ReadLevelFromFile(levelName);

    _cam = _level.FitCameraToBounds(1.0f, 16.0f, 9.0f);
}

void EditorManager::HandleAddingObjects()
{
    if (levelObjectsList.find(_selectedObjectId) == levelObjectsList.end()) return;
    const LevelObjectProperties& properties = levelObjectsList.at(_selectedObjectId);

    if (properties.canBePlacedByDragging)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && (_mousePrevX != _mouseX || _mousePrevY != _mouseY)))
            _level.EditorTryToPlaceObject(_mouseX, _mouseY, _selectedObjectId);
    }
    else
    {
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) _level.EditorTryToPlaceObject(_mouseX, _mouseY, _selectedObjectId);
    }
}

void EditorManager::HandleRemovingObjects()
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && (_mousePrevX != _mouseX || _mousePrevY != _mouseY)))
    {
        _level.EditorTryToRemoveObject(_mousePosition, _mouseX, _mouseY);
    }
}