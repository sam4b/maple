#pragma once
#include "Scene.hpp"
#include "GameScene.hpp"
#include "imgui-SFML.h"
#include "imgui.h"

[[nodiscard]] bool inBounds(const sf::Vector2i vector, int x, int y) noexcept {
    if (vector.x < 0 || vector.y < 0) return false;

    if (vector.x > x) return false;
    if (vector.y > y) return false;

    return true;
}

class MapWindow {
public:
    void setupWindow(Map* map) {
        this->map = map;
    }

    void update() {
        ImGui::Begin("Map");

        if (selectedLayer != -1) {
            ImGui::Text(std::format("Selected layer {}", selectedLayer).c_str());
        }

        if (ImGui::Button("+")) {
            if (map->getLayerCount() == 0) {
                selectedLayer = 0;
            }
            map->addLayer();
        }

        for (int i = 0; i < map->getLayerCount(); i++) {
            ImGui::Text(std::format("Layer {}", i).c_str());
            ImGui::SameLine();
            if (ImGui::Button(std::format("Select##{}", i).c_str())) {
                selectedLayer = i;
            }
            ImGui::SameLine();
            if (ImGui::Button(std::format("Delete##{}", i).c_str())) {
                if (selectedLayer >= i) {
                    selectedLayer--;
                }
                map->deleteLayer(i);
            }
        }

        ImGui::End();
    }

    void place(int x, int y, int id) {
        //log that a mistake happened but return
        assert(selectedLayer >= 0 && selectedLayer < map->getLayerCount());
        map->place(x, y, selectedLayer, id);
    }
private:
    Map* map;
    int selectedLayer = -1;
};

class TilesetWindow {
public:
    void setupWindow(const Tileset& tileset) {
        sprites.clear();

        for (int i = 0; i < tileset.tileCount(); i++) {
            const auto tile = tileset.idToSubTexture(i);
            sprites.emplace_back();
            sprites.back().setTexture(tileset.getTexture());
            sprites.back().setTextureRect(tile);
        }
    }

    void update(bool& open) {
        ImGui::Begin("Tileset");
        if (ImGui::Button("Close")) {
            open = false;
        }

        const sf::Vector2f drawSize = { 16,16 }; //We draw this at 16, 16 for a reasonable size, perhaps make it scale on window?
        for (int i = 0; i < sprites.size(); i++) {
            ImGui::PushID(i); //ImGui assigns an ID to each button based off the texture. Many buttons -> 1 texture... you get the idea. So just push the id.
            if (ImGui::ImageButton(sprites[i], drawSize)) {
                if (selected == i) { //Unselect if already selected
                    selected = unselected;
                }
                else { //Select for firs ttime
                    selected = i;
                }
            }
            ImGui::SameLine();
            if ((i + 1) % 16 == 0) { // (i + 1) zero hack.
                ImGui::NewLine();
            }
            ImGui::PopID();
        }
        ImGui::End();
    }

    [[nodiscard]] std::optional<int> getSelectedTile() const noexcept {
        if (selected == -1) {
            return std::optional<int>();
        }

        return std::optional<int>(selected);
    }

private:
    const int unselected = -1;
    int selected = unselected;
    std::vector<sf::Sprite> sprites;
};


sf::Vector2i getRelativeToLatestImGuiTopLeft(const sf::Vector2i mousePos) {
    const ImVec2 beginCorner = ImGui::GetCursorScreenPos();
    const sf::Vector2i corner = { (int)beginCorner.x, (int)beginCorner.y };

    const sf::Vector2i index = mousePos - corner;

    return index;
}

class SceneWindow {
public:
    void setupWindow(Scene* scene, TilesetWindow* tsWindow, MapWindow* mapWindow, sf::Vector2i size) {
        this->scene = scene;
        this->mapWindow = mapWindow;
        this->tsWindow = tsWindow;

        const int tileSize = 5;// scene->getMap().getTileSize().x;

        gridLine.reserve((size.y / tileSize) + (size.x / tileSize));

        
        for (int row = 0; row < size.y / tileSize;  row++) { //Deal w/ tile size
            const auto begin = sf::Vertex(sf::Vector2f(0, row * tileSize));
            const auto end = sf::Vertex(sf::Vector2f(size.x, row * tileSize));

            gridLine.push_back(begin);
            gridLine.push_back(end);
        }

        for (int col = 0; col < size.x / tileSize; col++) {
            const auto begin = sf::Vertex(sf::Vector2f(col * tileSize, 0));
            const auto end = sf::Vertex(sf::Vector2f(col * tileSize, size.y));

            gridLine.push_back(begin);
            gridLine.push_back(end);
        }
        texture.create(size.x, size.y);
        view = texture.getDefaultView();
        playing = false;

    }


    [[nodiscard]] bool isPlaying() const noexcept {
        return playing;
    }

    void update(const sf::Vector2i mousePos, std::queue <sf::Event>& outEvents) {
        ImGui::Begin("Scene", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        if (ImGui::Button("Play")) {
            if (!playing) playing = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Pause")) {
            if (playing) playing = false;
        }

        if (ImGui::Button("Resize View")) {
            view = texture.getDefaultView();
        }
        const auto vector = getRelativeToLatestImGuiTopLeft(mousePos);

        //Show tileset thingy under cursor. Opaque
        if (!playing && tsWindow->getSelectedTile().has_value()) {
            opt.setPosition({ (float)getRelativeToLatestImGuiTopLeft(mousePos).x, (float)getRelativeToLatestImGuiTopLeft(mousePos).y });
            opt.setSize({ 16, 16 });
            opt.setTextureRect(scene->map.getSet().idToSubTexture(tsWindow->getSelectedTile().value()));
            opt.setTexture(&scene->map.getSet().getTexture());
            auto c = opt.getFillColor();
            opt.setFillColor(c);
        }

        constexpr int IMGUI_SHOW_NO_BORDER_IMAGEBUTTON = 0;

        if (ImGui::ImageButton(texture, IMGUI_SHOW_NO_BORDER_IMAGEBUTTON)) {
            if (playing) { //Dispatch event to game.too
                sf::Event e;
                e.type = sf::Event::MouseButtonPressed;
                e.mouseButton.button = sf::Mouse::Left;
                e.mouseButton.x = mousePos.x;
                e.mouseButton.y = mousePos.y;

                outEvents.push(e);
            }
            else if (!playing) {
                if (tsWindow->getSelectedTile().has_value()) {
                    const int size = scene->map.getTileSize().x;
                    const auto x = vector.x / size;
                    const auto y = vector.y / size;
                    mapWindow->place(x, y, tsWindow->getSelectedTile().value());

                }
            }
        }

        ImGui::End();
    }

    void draw(MapleServices mapleView) {
        texture.clear();
        if (!playing) {
            texture.draw(gridLine.data(), gridLine.size(), sf::Lines);
        }
        RenderTarget out(&texture);
        scene->draw(out, mapleView);
        if (tsWindow->getSelectedTile().has_value() && !playing) {
            texture.draw({ opt });
        }
        texture.setView(view);
        texture.display();
    }
private:
    sf::View view;
    Scene* scene;
    MapWindow* mapWindow;
    TilesetWindow* tsWindow;
    sf::RectangleShape opt;
    sf::RenderTexture texture;
    std::vector<sf::Vertex> gridLine;
    bool playing;
};