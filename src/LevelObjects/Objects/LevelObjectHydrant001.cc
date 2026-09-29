#include "LevelObjectHydrant001.hh"

LevelObjectHydrant001::LevelObjectHydrant001(const Vector2& position, LevelObjectHydrant001Water* waterLeft, LevelObjectHydrant001Water* waterRight)
{
    _bounds = { position.x + (1.0f - kWidth) / 2.0f, position.y + (1.0f - kHeight), kWidth, kHeight };

    _waterLeft = waterLeft;
    _waterRight = waterRight;

    _isActive = false;
    _activeTimer = 0.0f;
}

void LevelObjectHydrant001::Update(const float deltaTime) 
{
    if (_isActive)
    {
        _activeTimer += deltaTime;
        if (_activeTimer >= kActiveDuration)
        {
            _activeTimer = 0.0f;
            Deactivate();
        }
    }
}

void LevelObjectHydrant001::Render(const float deltaTime) const 
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::Hydrant001);

    constexpr float kPixelsPerTile = 16.0f;

    Rectangle src = { 0.0f, 0.0f, 16.0f, 25.0f };
    const float spriteWidth  = src.width  / kPixelsPerTile;
    const float spriteHeight = src.height / kPixelsPerTile;

    const float groundY = _bounds.y + _bounds.height;

    Rectangle dst = {
        _bounds.x + _bounds.width / 2.0f - spriteWidth / 2.0f,
        groundY + (1.0f / kPixelsPerTile) - spriteHeight,
        spriteWidth,
        spriteHeight
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}

void LevelObjectHydrant001Water::RenderBounds(const float deltaTime, const Color& color) const
{
    if (!_isActive) return;
    LevelObject::RenderBounds(deltaTime, color);
}

void LevelObjectHydrant001::CollidedWithX(LevelObject* other, float prevVelocityX)
{
    if (other->GetVelocityMagnitude() >= kInteractionVelocity)
    {
        Activate();
    }
}

void LevelObjectHydrant001::CollidedWithY(LevelObject* other, float prevVelocityY)
{
    if (other->GetVelocityMagnitude() >= kInteractionVelocity)
    {
        Activate();
    }
}

void LevelObjectHydrant001::OnDug(LevelObject* digger)
{
    Activate();
}

void LevelObjectHydrant001::Activate()
{
    if (_isActive) return;

    _isActive = true;

    _waterLeft->Activate();
    _waterRight->Activate();
}

void LevelObjectHydrant001::Deactivate()
{
    if (!_isActive) return;

    _isActive = false;

    _waterLeft->Deactivate();
    _waterRight->Deactivate();
}

// ===== WATER =====

LevelObjectHydrant001Water::LevelObjectHydrant001Water(const Vector2& position, bool isLeft)
{
    const float anchorX = position.x + 0.5f + (isLeft ? -kHorizontalOffset : kHorizontalOffset);
    const float boundsX = isLeft ? (anchorX - kWidth) : anchorX;

    _bounds = { boundsX, position.y + (1.0f - kHeight) - kVerticalOffset, kWidth, kHeight };
    _isLeft = isLeft;
    _isActive = false;
}

void LevelObjectHydrant001Water::Update(const float deltaTime) 
{

}

void LevelObjectHydrant001Water::Render(const float deltaTime) const 
{
    if (!_isActive) return;

    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::Hydrant001Water);

    constexpr float kPixelsPerTile = 16.0f;
    constexpr float kSrcWidth = 21.0f;
    constexpr float kSrcHeight = 23.0f;

    // Amplada negativa del source = mirall horitzontal (truc estàndard de raylib)
    Rectangle src = { 0.0f, 0.0f, _isLeft ? -kSrcWidth : kSrcWidth, kSrcHeight };

    const float spriteWidth  = kSrcWidth  / kPixelsPerTile;
    const float spriteHeight = kSrcHeight / kPixelsPerTile;

    // Ancoratge horitzontal: el centre de _bounds JA és centre_hidrant +- kHorizontalOffset (veure constructor).
    // El sprite no es centra aquí, s'hi enganxa per un costat i creix cap enfora de l'hidrant.
    const float anchorX = _isLeft ? (_bounds.x + _bounds.width) : _bounds.x;
    const float dstX = _isLeft ? (anchorX - spriteWidth) : anchorX;

    // Vertical: centrat dins la hitbox
    const float dstY = _bounds.y + (_bounds.height - spriteHeight) * 0.5f;

    Rectangle dst = { dstX, dstY, spriteWidth, spriteHeight };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}

void LevelObjectHydrant001Water::Activate()
{
    _isActive = true;
}

void LevelObjectHydrant001Water::Deactivate()
{
    _isActive = false;
}

Vector2 LevelObjectHydrant001Water::GetForceFieldForce() const
{
    return { (_isLeft) ? -kForceFieldDirectionX : kForceFieldDirectionX, kForceFieldDirectionY };
}
