#pragma once
#include <vector>
#include <cstdint>
#include <MiniFB.h>

class SceneManager; // Forward declaration

class Scene {
public:
    virtual ~Scene() = default;
    
    // Core Lifecycle Hooks
    virtual void init(SceneManager& sm) = 0;
    virtual void update(struct mfb_window* window, SceneManager& sm, float dt) = 0;
    virtual void draw(std::vector<uint32_t>& buffer) = 0;
};
