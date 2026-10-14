#include <raylib.h>
#include <stdlib.h>

#include "GameManager.hh"

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600

int main(int argc, char* argv[])
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "DEEPER PROTOTYPE");

    SetExitKey(KEY_NULL);

    InitAudioDevice();
    SetTargetFPS(60);

    GameManager* gameManager = new GameManager();

    NetLaunchOptions netOptions;
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--host") netOptions.mode = NetLaunchOptions::Mode::Host;
        else if (arg == "--join" && i + 1 < argc) { netOptions.mode = NetLaunchOptions::Mode::Join; netOptions.address = argv[++i]; }
        else if (arg == "--lag" && i + 1 < argc) netOptions.lagMs = atoi(argv[++i]);
    }
    gameManager->SetNetLaunchOptions(netOptions);

    float accumulator = 0.0f;
    constexpr float kFixedDeltaTime = 1.0f / 60.0f;
    constexpr float kMaxFrameTime = 1.0f / 20.0f;

    while (!WindowShouldClose())
    {
        const float frameTime = std::min(GetFrameTime(), kMaxFrameTime);
        accumulator += frameTime;

        gameManager->PollInputToAddPlayers();
        gameManager->PollNetwork();

        while (accumulator >= kFixedDeltaTime)
        {
            gameManager->Update(kFixedDeltaTime);
            accumulator -= kFixedDeltaTime;
        }

        gameManager->Render(GetFrameTime());
    }

    delete gameManager;

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
