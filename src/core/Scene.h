#pragma once
#include <vector>
#include <cstddef>
#include <cstdint>
#include <MiniFB.h>
#include "Entity.h"
#include "Draw.h"
#include "Log.h"

class SceneManager; // Forward declaration

// Helper type for std::visit overloaded lambdas
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class Scene {
protected:
    std::vector<Entity> entities; // Common entity storage for any scene

public:
    uint32_t background_color = 0xFF000000;

    virtual ~Scene() = default;

    // Core Lifecycle Hooks
    virtual void init(SceneManager& sm) = 0;
    virtual void update(SceneManager& sm, float dt) = 0;

    // The core draw engine function called by SceneManager::draw.
    // Queues all entity and custom draw commands into the pipeline.
    // NOTE: flush_pipeline is intentionally NOT called here — SceneManager owns the flush.
    virtual void draw(std::vector<uint32_t>& screen_buffer) {
        // Automatically draw all managed entities sorted by z_index
        draw_entities(screen_buffer);

        // Hook for child scenes to do any extra custom drawing
        // (e.g., screen flashes, dynamic particle lines, raw pixel debugging)
        draw_custom(screen_buffer);
    }

protected:
    // Optional virtual hook so child scenes can override custom rendering if needed
    virtual void draw_custom(std::vector<uint32_t>& screen_buffer) {
        // Do nothing by default, but available if a scene needs raw rendering
    }

    // The shared built-in entity rendering pipeline
    void draw_entities(std::vector<uint32_t>& screen_buffer) {
        for (const auto& entity : entities) {
            if (!entity.active) continue;

            // Use a lambda overload pattern for std::visit
            std::visit(overloaded {
                [&](const RectangleRender& visual_data) {
                    Draw::rect(
                        (int)entity.transform.x,
                        (int)entity.transform.y,
                        (int)entity.transform.width,
                        (int)entity.transform.height,
                        visual_data.color,
                        visual_data.fill,
                        entity.transform.z_index
                    );
                },
                [&](const SpriteRender& visual_data) {
                    Draw::sprite(
                        (int)entity.transform.x,
                        (int)entity.transform.y,
                        visual_data.pixels,
                        visual_data.pixels_size,
                        visual_data.width,
                        visual_data.height,
                        entity.transform.z_index
                    );
                },
                [&](const TextRender& visual_data) {
                    Draw::text(
                        (int)entity.transform.x,
                        (int)entity.transform.y,
                        visual_data.text,
                        visual_data.color,
                        visual_data.scale,
                        entity.transform.z_index
                    );
                }
            }, entity.visual);
        }
    }

    // Returns the index of the first entity with a matching tag.
    // Prefer caching this in init() rather than calling every frame.
    //
    // NOTE: indices (not pointers/references) are the correct long-term handle
    // for an entity in a std::vector — they survive reallocation safely.
    // If no match is found, returns SIZE_MAX as a sentinel (check before use).
    //
    // --- REMOVING / DESPAWNING ENTITIES (e.g. bullets, enemies) ---
    // Removing elements from `entities` invalidates cached indices for any
    // entity that sits AFTER the removed element in the vector (they all shift
    // left by one). There are three practical strategies, pick by use-case:
    //
    // 1. SOFT-DELETE via `entity.active = false`  <-- RECOMMENDED for most games
    //    Don't erase at all. Just flip `entity.active = false` to stop it from
    //    being drawn or updated. draw_entities() already skips inactive entries.
    //    Indices are NEVER invalidated. Recycle slots later if memory matters.
    //    Best for bullets, enemies, particles — anything with high churn.
    //
    // 2. SWAP-AND-POP (erase without shifting)
    //    To truly remove element at index i without shifting others:
    //      std::swap(entities[i], entities.back());
    //      entities.pop_back();
    //    Only the swapped entity (previously at .back()) changes index.
    //    If you hold a cached index to it, re-assign: idx = i after the swap.
    //    Order of entities in the vector is NOT preserved (z_index handles draw order).
    //
    // 3. LINEAR ERASE + RE-CACHE (simple but O(n) per removal)
    //    Call entities.erase(entities.begin() + i), then call entity_index(tag)
    //    again for every cached index you hold. Simple, correct, slow for bulk.
    //    Fine for infrequent removals (boss death, level transition, etc.).
    //
    // --- SCALING UP (if soft-delete isn't enough) ---
    // If the vector grows unboundedly (thousands of bullets/particles), recycle
    // inactive slots instead of always pushing new ones — this is an Object Pool.
    // Scan for the first inactive entry in spawn logic and reuse it; only push_back
    // if none are free. The vector grows to a high-water mark then plateaus,
    // giving you zero heap allocations after warm-up.
    //
    // For AAA-scale needs, look up "Generational Indices" (also called Entity Handles).
    // Used by engines like Unreal, Unity DOTS, and EnTT — worth a search or AI chat.
    size_t entity_index(const std::string& tag) const {
        for (size_t i = 0; i < entities.size(); ++i) {
            if (entities[i].tag == tag) return i;
        }
        return SIZE_MAX; // sentinel: no entity found
    }
};
