#pragma once
#include "Scene.hpp"
#include "GameScene.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "Windows.hpp"

class EditorScene : public Scene {
public:
    EditorScene(Scene* scene) {
        this->scene = scene;
    }

    void onSceneCreate(MapleServices view, const nlohmann::json& context) {
        scene->onSceneCreate(view, context);
        tsWindow.setupWindow(scene->map.getSet());
        mapWindow.setupWindow(&scene->map); //kinda evil

        sceneWindow.setupWindow(scene, &tsWindow, &mapWindow, scene->getDrawSize());
    }

    ~EditorScene() override {
        scene->save();
    }

    sf::Vector2i getDrawSize() const noexcept override {
        return { 1280, 720 };
    }

    void update(std::queue<sf::Event> events, const sf::Vector2i mousePos, const float dt, MapleServices& view, const sf::Time time) noexcept override {
        std::queue<sf::Event> realSceneEvents;
        while (!events.empty()) {
            const auto top = events.front();
            const bool editorShouldCapture = false;
                if (top.type == sf::Event::MouseButtonPressed || top.type == sf::Event::MouseButtonReleased) {
                    //ImGui reinjects *if* we want this event. (See SceneWindow::Update).
                }
                else if (sceneWindow.isPlaying()) {
                    realSceneEvents.push(top);
                }
            events.pop();
        }
       
//        tsWindow.update();
        mapWindow.update();

//        entityWindow.update(*view.entityManager);

        /*Scene window re-adds events filtered out by ImGui (because we render the Scene to an ImGui::Image.*/
        sceneWindow.update(mousePos, realSceneEvents);

        if (sceneWindow.isPlaying()) {
            scene->update(realSceneEvents, mousePos, dt, view, time);
        }
    }
    

    void draw(RenderTarget& target, MapleServices view) noexcept override {
        sceneWindow.draw(view);
    }

    void save() const noexcept override {
        //Save editor settings?
    }

private:
    Scene* scene;
    SceneWindow sceneWindow;
    TilesetWindow tsWindow;
    MapWindow mapWindow;
};