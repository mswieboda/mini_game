#pragma once
#include <memory>
#include "Scene.h"

class SceneManager {
private:
    std::unique_ptr<Scene> m_currentScene;
    std::unique_ptr<Scene> m_nextScene;

public:
    void changeScene(std::unique_ptr<Scene> newScene) {
        // Queue the scene change to avoid deleting a scene mid-update loop
        m_nextScene = std::move(newScene);
    }

    void processPendingChanges() {
        if (m_nextScene) {
            m_currentScene = std::move(m_nextScene);
            m_currentScene->init(*this);
        }
    }

    void update(struct mfb_window* window, float dt) {
        if (m_currentScene) m_currentScene->update(window, *this, dt);
    }

    void draw(std::vector<uint32_t>& buffer) {
        if (m_currentScene) m_currentScene->draw(buffer);
    }
};
