#include "Effects.h"
#include <cmath>

Particle::Particle(Vector2 pos, Vector2 vel, float s, Color c, float life) {
    position = pos;
    velocity = vel;
    size = s;
    color = c;
    lifespan = life;
    maxLife = life;
    alpha = 1.0f;
}

void Particle::Update(float deltaTime) {
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
    lifespan -= deltaTime;
    alpha = lifespan / maxLife;
}

void Particle::Draw() {
    Color currentColor = color;
    currentColor.a = (unsigned char)(alpha * 255);
    DrawCircleV(position, size, currentColor);
}

ParticleSystem::ParticleSystem(int screenWidth, int screenHeight, int maxP) {
    maxParticles = maxP;
    backgroundTexture = LoadRenderTexture(screenWidth, screenHeight);
    time = 0;
}

ParticleSystem::~ParticleSystem() {
    UnloadRenderTexture(backgroundTexture);
}

void ParticleSystem::Update(float deltaTime) {
    time += deltaTime;

    // Update existing particles
    for (auto it = particles.begin(); it != particles.end();) {
        it->Update(deltaTime);
        if (it->lifespan <= 0) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }

    // Create new particles randomly
    if (GetRandomValue(0, 100) < 5 && particles.size() < maxParticles) {
        Vector2 pos = {
            (float)GetRandomValue(0, GetScreenWidth()),
            (float)GetRandomValue(0, GetScreenHeight())
        };
        CreateParticle(pos);
    }
}

void ParticleSystem::CreateParticle(Vector2 position) {
    if (particles.size() >= maxParticles) return;

    Vector2 vel = {
        (float)GetRandomValue(-50, 50) / 100.0f,
        (float)GetRandomValue(-50, 50) / 100.0f
    };
    float size = GetRandomValue(2, 4);
    Color color = {
        (unsigned char)GetRandomValue(40, 80),
        (unsigned char)GetRandomValue(100, 200),
        (unsigned char)GetRandomValue(200, 255),
        255
    };
    float life = GetRandomValue(3, 6);
    
    particles.push_back(Particle(position, vel, size, color, life));
}

void ParticleSystem::DrawBackground(float deltaTime) {
    BeginTextureMode(backgroundTexture);
        // Draw gradient background
        for (int y = 0; y < GetScreenHeight(); y += 2) {
            float factor = (float)y / GetScreenHeight();
            Color color = {
                (unsigned char)(20 + factor * 10),
                (unsigned char)(30 + factor * 10),
                (unsigned char)(40 + factor * 10),
                255
            };
            DrawRectangle(0, y, GetScreenWidth(), 2, color);
        }

        // Draw animated wave pattern
        for (int x = 0; x < GetScreenWidth(); x += 4) {
            float y = sin(x * 0.02f + time) * 20 + GetScreenHeight() / 2;
            DrawCircle(x, y, 1, (Color){67, 134, 245, 30});
        }
    EndTextureMode();
}

void ParticleSystem::Draw() {
    // Draw background
    DrawTexture(backgroundTexture.texture, 0, 0, WHITE);
    
    // Draw all particles
    for (auto& particle : particles) {
        particle.Draw();
    }
}
