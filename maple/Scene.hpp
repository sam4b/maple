#pragma once
#include <queue>
#include <SFML/Graphics.hpp>
#include <cassert>
#include <iostream>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "RenderTarget.hpp"
#include "Scripting.hpp"
#include <ranges>
#include <imgui.h>
#include "ScriptManager.hpp"
#include "EntityManager.hpp"
#include "Entity.hpp"
#include <format>

class Scene {
public:
    virtual void draw(RenderTarget& target, Systems view) noexcept = 0;
    virtual void update(std::queue<sf::Event> events, const sf::Vector2i mousePos, const float dt, Systems& view, const sf::Time time) noexcept = 0;
    virtual ~Scene() = default;
    virtual sf::Vector2i getDrawSize() const noexcept = 0;


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