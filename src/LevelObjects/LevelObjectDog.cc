#include "LevelObjectDog.hh"
#include "LevelManager.hh"

LevelObjectDog::LevelObjectDog(const Vector2& position, LevelManager& level)
    : LevelObject(position), _level(level)
{
    _bounds = { position.x, position.y, kWidth, kHeight };
}

void LevelObjectDog::Update(const float deltaTime) 
{
    ApplyGravity(deltaTime);
    MoveAndCollide(deltaTime);
}

void LevelObjectDog::Render(const float deltaTime) const {
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::DogTest);

    const float spriteSize = 2.0f;

    Rectangle src = { 0.0f, 0.0f, 32.0f, 32.0f };
    Rectangle dst = {
        _bounds.x + _bounds.width / 2.0f - spriteSize / 2.0f,
        _bounds.y + _bounds.height / 2.0f - spriteSize / 2.0f,
        spriteSize,
        spriteSize
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
    DrawRectangleLinesEx(_bounds, 0.05f, ORANGE);
}

void LevelObjectDog::ApplyGravity(const float deltaTime) 
{
    _velocity.y += kGravity * deltaTime;
    if (_velocity.y > kMaxFallSpeed) _velocity.y = kMaxFallSpeed;
}

void LevelObjectDog::MoveAndCollide(const float deltaTime) 
{
    _isGrounded = false;

    _bounds.x += _velocity.x * deltaTime;
    ResolveTilesX();

    _bounds.y += _velocity.y * deltaTime;
    ResolveTilesY();
}

void LevelObjectDog::ResolveTilesX() 
{
    int tileLeft = (int)std::floor(_bounds.x);
    int tileRight = (int)std::floor(_bounds.x + _bounds.width);
    int tileTop = (int)std::floor(_bounds.y);
    int tileBottom = (int)std::floor(_bounds.y + _bounds.height - 0.01f);

    for (int ty = tileTop; ty <= tileBottom; ++ty) 
    {
        for (int tx = tileLeft; tx <= tileRight; ++tx) 
        {
            if (_level.GetTileTypeAt({ (float)tx, (float)ty }) != LevelObjectTileType::Ground)
                continue;

            Rectangle tileBounds = { (float)tx, (float)ty, 1.0f, 1.0f };
            Rectangle overlap = GetCollisionRec(_bounds, tileBounds);
            if (overlap.width <= 0) continue;

            if (_velocity.x > 0)
                _bounds.x -= overlap.width;
            else if (_velocity.x < 0)
                _bounds.x += overlap.width;

            _velocity.x = 0;
        }
    }
}

void LevelObjectDog::ResolveTilesY() {
    int tileLeft = (int)std::floor(_bounds.x + 0.01f);
    int tileRight = (int)std::floor(_bounds.x + _bounds.width - 0.01f);
    int tileTop = (int)std::floor(_bounds.y);
    int tileBottom = (int)std::floor(_bounds.y + _bounds.height);

    for (int ty = tileTop; ty <= tileBottom; ++ty) 
    {
        for (int tx = tileLeft; tx <= tileRight; ++tx) 
        {
            if (_level.GetTileTypeAt({ (float)tx, (float)ty }) == LevelObjectTileType::None)
                continue;

            Rectangle tileBounds = { (float)tx, (float)ty, 1.0f, 1.0f };
            Rectangle overlap = GetCollisionRec(_bounds, tileBounds);
            if (overlap.height <= 0) continue;

            if (_velocity.y > 0) {
                _bounds.y -= overlap.height;
                _isGrounded = true;
            }
            else if (_velocity.y < 0) {
                _bounds.y += overlap.height;
            }

            _velocity.y = 0;
        }
    }
}