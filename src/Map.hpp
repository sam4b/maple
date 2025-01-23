#pragma once
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include "Tileset.hpp"
#include "MapleComponents.hpp"
#include "RenderTarget.hpp"

struct GridEntry {
    sf::Vector2i pos;
    int id;
    sf::RectangleShape shape;
    bool collidable;
};

class Map {
public:

    void addLayer() {
        layers.push_back({});
    }


    //World coords ->
    bool isCollidable(AABBCollisionComponent collider) {
        const int x = collider.pos.x / tileSize;
        const int y = collider.pos.y / tileSize;

        for (const auto& layer : layers) {
            for (const auto& entry : layer) {
                if (entry.pos.x == x && entry.pos.y == y && entry.collidable) return true;
            }
        }
        return false;
    }

    void deleteLayer(int index) {
        assert(index >= 0);
        assert(index < layers.size());
        layers.erase(layers.begin() + index);
    }


    int getLayerCount() {
        return layers.size();
    }

    void loadFrom(const nlohmann::json& json) {

        const int tilesetID = json["tilesetID"];

        this->set = loadTileset(tilesetID);

        const int tileSize = json["tileSize"];
        this->tileSize = tileSize;
        for (const auto& layer : json["layers"]) {
            layers.push_back(std::vector<GridEntry>());


            std::vector<GridEntry>& entries = layers.back();

            for (const auto& rect : layer) {
                sf::RectangleShape r;


                assert(rect["id"].is_number_integer());
                assert(rect["x"].is_number_integer());
                assert(rect["y"].is_number_integer());
                assert(rect["collidable"].is_boolean());
                const int id = rect["id"];
                r.setTexture(&set.getTexture());
                r.setTextureRect(set.idToSubTexture(id));
                r.setSize({ (float) tileSize, (float) tileSize });
                r.setPosition({ (float)rect["x"] * tileSize, (float)rect["y"] * tileSize }); //Needs to be changed from relative to window saved in to relative to grid.
                
                GridEntry entry;
                entry.id = id;
                entry.pos = { rect["x"], rect["y"] };
                entry.shape = r;
                entry.collidable = rect["collidable"];
                entries.push_back(entry);
            }

        }
    }

    Tileset& getSet() {
        return set;
    }

    sf::Vector2i getTileSize() const noexcept {
        return { tileSize, tileSize };
    }

    void place(int x, int y, int selectedLayer, int id) {
        if (selectedLayer >= layers.size() || selectedLayer < 0) {
            assert(false);
        }


        // assert(layers[selectedLayer].shapes.size() == layers[selectedLayer].ids.size());
        for (int i = 0; i < layers[selectedLayer].size(); i++) {
            GridEntry& entry = layers[selectedLayer][i];
            if (entry.pos == sf::Vector2i{x, y}) {
                entry.shape.setTextureRect(set.idToSubTexture(id));
                entry.id = id;
                return;
            }
        }


        sf::RectangleShape r;
        r.setTexture(&set.getTexture());
        r.setTextureRect(set.idToSubTexture(id));
        r.setSize({ (float) tileSize, (float) tileSize });
        r.setPosition({ (float) x * tileSize, (float) y * tileSize });
        GridEntry entry;
        entry.id = id;
        entry.pos = { x, y };
        entry.shape = r;
       
        layers[selectedLayer].push_back(entry);
    }

    nlohmann::json toJson() const noexcept {
        nlohmann::json json;
        for (const auto& layer : layers) {
            nlohmann::json rects;


            for (int i = 0; i < layer.size(); i++) {
                nlohmann::json rect;
                rect["x"] = layer[i].pos.x;
                rect["y"] = layer[i].pos.y;
                rect["id"] = layer[i].id;
                rect["collidable"] = layer[i].collidable;
                rects.push_back(rect);
            }
            json["layers"].push_back(rects);
            json["tilesetID"] = set.getTilesetID();
            json["tileSize"] = tileSize;
        }

        return json;
    }

    void draw(RenderTarget& target) const noexcept {
        for (const auto& layer : layers) {
            for (const auto& entry : layer) {
                target.draw(entry.shape);
            }
        }
    }
private:
    int tileSize; //uniform
    std::vector<std::vector<GridEntry>> layers;
    Tileset set;
};