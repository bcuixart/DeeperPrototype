#include <raylib.h>
#include <stdlib.h>

#include "GameManager.hh"

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600

int main(int argc, char* argv[])
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    if (argc == 3) 
    {
		int wdth = atoi(argv[1]);
		int hght = atoi(argv[2]);

        InitWindow(wdth, hght, "DEEPER PROTOTYPE");
    }
    else 
    {
        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "DEEPER PROTOTYPE");
    }

    SetExitKey(KEY_NULL);

    InitAudioDevice();
    SetTargetFPS(60);

    GameManager* gameManager = new GameManager();

    bool _firstFrame = true;
    float deltaTime = 0;
    while (!WindowShouldClose())
    {
        if (_firstFrame) { _firstFrame = false; continue; }
        deltaTime = std::min(GetFrameTime(), 1.0f / 20.0f);

        gameManager->Update(deltaTime);
        gameManager->Render(deltaTime);
    }

    delete gameManager;

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
