#include "Core/Engine.hpp"

class ParticleSystem {
public:
private:
    Engine engine;

public:
    ParticleSystem() = default;
    ~ParticleSystem() = default;
    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

private:
};