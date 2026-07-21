#include "Scene.h"
#include "Entity.h"
#include "Font.h"

// Helper type for std::visit overloaded lambdas
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void Scene::draw(std::vector<uint32_t>& screen_buffer) {
    draw_entities(screen_buffer);
    draw_custom(screen_buffer);
}

void Scene::draw_custom(std::vector<uint32_t>& screen_buffer) {
    // Optional implementation hook; blank by default
}

void Scene::update_entities(float dt) {
    for (auto& entity : entities) {
        if (!entity.active) continue;

        // Process animations if the entity uses AnimatedSpriteRender
        if (auto* anim = std::get_if<AnimatedSpriteRender>(&entity.visual)) {
            if (!anim->is_playing || anim->current_anim.frame_indices.empty()) {
                continue;
            }

            // Convert dt seconds to milliseconds
            anim->elapsed_time_ms += dt * 1000.0f;

            // Loop to catch up on frames (handles slow frames or long lags smoothly)
            while (true) {
                int frame_pool_index = anim->current_anim.frame_indices[anim->current_sequence_index];
                const SpriteFrame& current_frame = anim->master_frames[frame_pool_index];

                if (anim->elapsed_time_ms < current_frame.duration_ms) {
                    break; 
                }

                anim->elapsed_time_ms -= current_frame.duration_ms;
                anim->current_sequence_index++;

                // Bound check / animation wrapping rules
                if (anim->current_sequence_index >= anim->current_anim.frame_indices.size()) {
                    if (anim->current_anim.loop) {
                        anim->current_sequence_index = 0;
                    } else {
                        // Stay clamped to the last frame index if loop is false
                        anim->current_sequence_index = anim->current_anim.frame_indices.size() - 1;
                        anim->is_playing = false;
                        break;
                    }
                }
            }
        }
    }
}

void Scene::draw_entities(std::vector<uint32_t>& screen_buffer) {
    for (const auto& entity : entities) {
        if (!entity.active) continue;

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
            [&](const AnimatedSpriteRender& visual_data) {
                if (visual_data.current_anim.frame_indices.empty()) return;

                // Extract source rect coordinates from the current animation index pool
                int frame_pool_index = visual_data.current_anim.frame_indices[visual_data.current_sequence_index];
                const SpriteFrame& current_frame = visual_data.master_frames[frame_pool_index];

                // Submit sub-rect slice coordinates to the pipeline queue
                Draw::sprite_frame(
                    (int)entity.transform.x,
                    (int)entity.transform.y,
                    visual_data.sheet_pixels,
                    visual_data.sheet_pixels_size,
                    visual_data.sheet_width,
                    visual_data.sheet_height,
                    current_frame.x,
                    current_frame.y,
                    current_frame.width,
                    current_frame.height,
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
                    entity.transform.z_index,
                    visual_data.font
                );
            }
        }, entity.visual);
    }
}

size_t Scene::entity_index(const std::string& tag) const {
    for (size_t i = 0; i < entities.size(); ++i) {
        if (entities[i].tag == tag) return i;
    }
    return SIZE_MAX;
}
