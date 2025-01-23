#pragma once
#include <queue>
#include <SFML/Graphics.hpp>
#include <cassert>
#include <iostream>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "Map.hpp"
#include "RenderTarget.hpp"
#include "Scripting.hpp"
#include <ranges>
#include <imgui.h>
#include "ScriptManager.hpp"
#include "EntityManager.hpp"
#include "Entity.hpp"
#include <format>

class TextRenderer {
public:
    TextRenderer(std::string&& text, int millis) : ptr(0), text(std::move(text)), accum(0.0f), micros(millis * 1000) {};

    [[nodiscard]] bool finished() const noexcept {
        return ptr == text.size();
    }

    void update(sf::Text& text, const sf::Time time) {
        if (!finished()) {
            accum += time.asMicroseconds();
            if (accum >= (micros / this->text.size())) {
                ptr++;
                text.setString(this->text.substr(0, ptr)); //Lazy update.
                accum = 0;
            }
        }
    }
private:
    int ptr;
    int accum;
    int micros;
    std::string text;
};

struct DialogBox {
    int elapsedSinceUpdate;
    int ptr;
    int durationPerCharacter;
    std::string data;
    sf::Text text;
    sf::RectangleShape rectangle;
};

DialogBox create_dialog_box(sf::Vector2f size, sf::Vector2f pos, const std::string& string, int durationInMilliseconds, sf::Font& font) {
    sf::RectangleShape shape;
    shape.setFillColor(sf::Color::Black);
    shape.setPosition(pos);
    shape.setSize(size);

    sf::Text text;
    text.setCharacterSize(12);
    text.setFont(font);
    text.setPosition(pos);
    text.setFillColor(sf::Color::White);

    DialogBox box;
    box.text = text;
    box.data = string;
    box.rectangle = shape;
    box.durationPerCharacter = (durationInMilliseconds * 1000 /*Scale to microseconds*/) / string.size();
    box.ptr = 0;
    box.elapsedSinceUpdate = 0;

    return box;
}

void update_dialog_box(DialogBox* box, bool* finished, sf::Time time) {
    if (*finished) return;

    if (box->ptr != box->data.size()) {
        box->elapsedSinceUpdate += time.asMicroseconds();
        if (box->durationPerCharacter <= box->elapsedSinceUpdate) {
            box->ptr++;
            const std::string sub = box->data.substr(0, box->ptr);
            //std::cout << sub << std::endl;
            box->text.setString(sub); //Lazy update.
            box->elapsedSinceUpdate = 0;
        }
        *finished = false;
    }
    else {
        *finished = true;
    }
}
class Scene {
public:
    virtual void draw(RenderTarget& target, Systems view) noexcept = 0;
    virtual void update(std::queue<sf::Event> events, const sf::Vector2i mousePos, const float dt, Systems& view, const sf::Time time) noexcept = 0;
    virtual ~Scene() = default;
    virtual sf::Vector2i getDrawSize() const noexcept = 0;
    DialogBox box;
    bool boxFinished = true;
    sf::Font font;

    Map map;




    bool collision(AABBCollisionComponent a, AABBCollisionComponent b) {
       return a.pos.x < b.pos.x + b.size.x
            &&
            a.pos.x + a.size.x > b.pos.x&&
            a.pos.y < b.pos.y + b.size.y &&
            a.pos.y + a.size.y > b.pos.y;
    }


    bool moveWouldCauseCollision(Entity entity, sf::Vector2f velocity, float dt, Systems context) {
        if (velocity == sf::Vector2f{0, 0}) return false;
        assert(entity.hasComponent<AABBCollisionComponent>());

        AABBCollisionComponent collider = entity.getComponent<AABBCollisionComponent>();
        collider.pos += (dt * velocity);


        //Check tilemap collision
        if (map.isCollidable(collider)) {
            return true;
        }



         for (Entity other : getEntities(context)) {
             if (other.getID() != entity.getID() && other.hasComponent<AABBCollisionComponent>()) {
                 const auto& otherCollider = other.getComponent<AABBCollisionComponent>();
                 ImGui::Begin("AABB Debug");
                 ImGui::Text(std::format("Pos A: {}, {}", collider.pos.x, collider.pos.y).c_str());
                 ImGui::Text(std::format("Size A: {}, {}", collider.size.x, collider.size.y).c_str());
                 ImGui::Text(std::format("Pos B: {}, {}", otherCollider.pos.x, otherCollider.pos.y).c_str());
                 ImGui::Text(std::format("Size B: {}, {}", otherCollider.size.x, otherCollider.size.y).c_str());
                 ImGui::End();
                 if (collision(collider, otherCollider)) {
                     return true;
                 }
             }
         }

        return false;
    }

    virtual void onSceneCreate(Systems view, const nlohmann::json& savedData) = 0;

    void putDialog(const std::string& text, int amount) {
        assert(font.loadFromFile("C:/Users/Sam/Downloads/Arial.ttf"));
            box = create_dialog_box({ 512, 256 }, { 0, 0 }, text, amount, font);
        boxFinished = false;
    }

    std::vector<Entity> getEntities(Systems view) noexcept {
        assert(view.entityManager);

        std::vector<Entity> out;
        out.reserve(view.entityManager->getEntities().size());
        for (uint64_t id : view.entityManager->getEntities()) {
            out.push_back(*(Entity*)&id); //hehe
        }
        return out;
    }

    /*
    Caller must have a TransformComponent.
    It's also mandated that any entities returned have one. If an entity you expect is not appearing - it's probably that.
    */
    template <typename ...Components>
    std::vector<Entity> getEntitiesNear(Entity caller, Systems view, float radius) noexcept {
        static_assert(sizeof...(Components) > 0, "At least one component type is required.");
        EntityManager* manager = view.entityManager;

        assert(manager);
        assert(caller.hasComponent<TransformComponent>());

        const TransformComponent callerTransform = caller.getComponent<TransformComponent>();

        std::vector<Entity> entities;
        for (Entity entity : getEntities(view)) {
            if (entity.getID() == caller.getID()) continue;
             
            if (!(entity.hasComponent<Components>() && ...)) continue;

            const TransformComponent entityTransform = entity.getComponent<TransformComponent>();

            const float distance = std::sqrt(std::pow(callerTransform.pos.x - entityTransform.pos.x, 2) + std::pow(callerTransform.pos.y - entityTransform.pos.y, 2));

            if (distance > radius) continue;

            entities.push_back(entity);
        }

        return entities;
    }
    /*If you override scene, you must implement this to allow for the editor to function with your custom scene.*/
    virtual void save() const noexcept = 0;

};