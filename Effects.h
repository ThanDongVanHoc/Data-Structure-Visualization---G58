#pragma once
#include "raylib.h"
#include <vector>

class Particle {
public:
    Vector2 position;
    Vector2 velocity;
    float size;
    float alpha;
    Color color;
    float lifespan;
    float maxLife;

    Particle(Vector2 pos, Vector2 vel, float s, Color c, float life);
    void Update(float deltaTime);
    void Draw();
};

class ParticleSystem {
private:
    std::vector<Particle> particles;
    int maxParticles;
    RenderTexture2D backgroundTexture;
    float time;

public:
    ParticleSystem(int screenWidth, int screenHeight, int maxP = 100);
    ~ParticleSystem();
    void Update(float deltaTime);
    void Draw();
    void CreateParticle(Vector2 position);
    void DrawBackground(float deltaTime);
};
