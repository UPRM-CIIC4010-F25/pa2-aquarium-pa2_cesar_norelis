#pragma once
#include "Core.h"

enum class PowerUpType {
    SPEED,
    SIZE
};

class PowerUp : public Creature {
public:
    PowerUp(float x, float y, PowerUpType type, std::shared_ptr<GameSprite> sprite)
        : Creature(x, y, 0, 25.0f, 0, std::move(sprite)), m_type(type) {}

    void move() override {} // Power-up stays in place (or could float later)

    void draw() const override {
        if (m_sprite) m_sprite->draw(m_x, m_y);
    }

    PowerUpType getType() const { return m_type; }

      void update() {
        // Optional: add floating animation or effects here
    }

private:
    PowerUpType m_type;
};
