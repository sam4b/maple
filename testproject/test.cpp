#include <maple.hpp>
/*
    Temp here, should go in engine
*/
void AddAnimation(const uint64_t uuid, Entity entity, const Systems systems) {
    assert(!entity.hasComponent<AnimationStateComponent>());
    const auto& animation = systems.assetManager->GetAnimation(uuid);
    auto& state = entity.addComponent<AnimationStateComponent>();

    state.animationID = uuid;
    state.offset = 0;
    state.lastUpdate = animation.frameTime;
    state.priorTexture = entity.getComponent<SpriteComponent>().texture;
    state.playForever = false;
}

void AddAnimation(const std::string& animation, Entity entity, const Systems systems) {
    assert(entity.hasComponent<SpriteComponent>());
    assert(entity.hasComponent<TransformComponent>());
    const auto properties = systems.assetManager->GetRegistry().getProperties(animation);
    assert(properties.type == AssetProperties::Type::Animation);
    AddAnimation(properties.uuid, entity, systems);
}


struct PossessedComponent : public ComponentMetadata {
    uint64_t posessor;
    int timeInMillis;

    // Inherited via ComponentMetadata
    void FromJson(const nlohmann::json& json, Systems context) noexcept override
    {
    }
    nlohmann::json ToJson() const noexcept override
    {
        return nlohmann::json();
    }
};
REGISTER_COMPONENT(PossessedComponent);

class BatScript : public Script {
    // Inherited via Script
    void onAttach(Systems& manager) override
    {
        auto& trans = entity.addComponent<TransformComponent>();
        auto& sprite = entity.addComponent<SpriteComponent>();
        
        trans.pos = { 256, 0 };
        trans.velocity = { 0, 0 };

        sprite.rectangle.setFillColor(sf::Color::Blue);
        sprite.rectangle.setPosition(trans.pos);
        sprite.rectangle.setSize({ 64, 64 });

        Physics2DComponent& physics = entity.addComponent<Physics2DComponent>();
        physics.mass = 0;

        AABBCollisionComponent& collider = entity.addComponent<AABBCollisionComponent>();
        collider.pos = trans.pos;
        collider.size = { 64, 64 };

    }
    void onDetach() override
    {
    }
    void onUpdate(const float dt, Systems& view, Scene* scene) override
    {
        ImGui::Begin("Bat");
        ImGui::DragFloat("Mass", &entity.getComponent<Physics2DComponent>().mass);
        ImGui::End();
        if (entity.hasComponent<PossessedComponent>()) {
            ImGui::Begin("Possession Window");
            auto& trans = entity.getComponent<TransformComponent>();
            auto& possessed = entity.getComponent<PossessedComponent>();
            possessed.timeInMillis -= dt * 100;
            ImGui::Text(std::to_string(possessed.timeInMillis).c_str());
            ImGui::End();
            if (possessed.timeInMillis <= 0) {
                //tg
                auto& storage = *view.entityManager;
                view.entityManager->removeComponent<PossessedComponent>(entity.getID());
            }
            else {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
                    trans.velocity = { -100 , 0 };
                }
            }
        }
    }
};
REGISTER_SCRIPT(BatScript);

class SylvanScript : public Script {
    void onAttach(Systems& manager) override
    {
        const auto texture = manager.assetManager->GetTexture("sylvan_spritesheet_3").value();

        SpriteComponent& sprite = entity.addComponent<SpriteComponent>();
        sprite.rectangle.setTexture(texture.texture);
        sprite.rectangle.setTextureRect(texture.rect);
        sprite.rectangle.setSize({ 66, 66 });
        sprite.rectangle.setPosition({ 0, 0 });

        TransformComponent& trans = entity.addComponent<TransformComponent>();
        trans.pos = { 0, 0 };
        trans.velocity = { 0, 0 };

        Physics2DComponent& physics = entity.addComponent<Physics2DComponent>();
        physics.mass = 30;

        AABBCollisionComponent& collider = entity.addComponent<AABBCollisionComponent>();
        collider.pos = trans.pos;
        collider.size = { 66, 66 };
    }
    void onDetach() override
    {
    }
    void onUpdate(const float dt, Systems& view, Scene* scene) override
    {
        bool iterated = false;
        for (const auto& a : scene->getEntities(view) | std::ranges::views::filter([&](Entity e) -> bool { return e.hasComponent<PossessedComponent>(); })) {
            iterated = true;
        }
        if (iterated) return;

        auto& trans = entity.getComponent<TransformComponent>();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            if (!entity.hasComponent<AnimationStateComponent>()) {
              //  AddAnimation("sylvan_walk", entity, view);
            }
            trans.velocity += {100, 0};
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            trans.velocity += {0, -500};
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            //play flipped
            trans.velocity -= {100, 0};
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P)) {
            auto vec = scene->getEntitiesNear<TransformComponent>(entity, view, 10000.0f);
            auto& posession = vec[0].addComponent<PossessedComponent>();
            posession.posessor = entity.getID();
            posession.timeInMillis = 500;
        }
    }
};
REGISTER_SCRIPT(SylvanScript);

class SylvanScene : public Scene {
    void draw(RenderTarget& target, Systems view) noexcept override
    {
    }
    void update(std::queue<sf::Event> events, const sf::Vector2i mousePos, const float dt, Systems& view, const sf::Time time) noexcept override
    {
   

    }
    sf::Vector2i getDrawSize() const noexcept override
    {
        return sf::Vector2i();
    }
    void onSceneCreate(Systems view, const nlohmann::json& savedData) override
    {

        Entity entity = Entity::createEntity();

        view.scriptManager->addScript<SylvanScript>(entity, view);
        entity.addComponent<NameComponent>().name = "Sylvan";

        Entity enemy = Entity::createEntity();
        view.scriptManager->addScript<BatScript>(enemy, view);
        enemy.addComponent<NameComponent>().name = "Bat";
        {
            Entity floor = Entity::createEntity();
            auto& transform = floor.addComponent<TransformComponent>();
            auto& sprite = floor.addComponent<SpriteComponent>();
            auto& aabb = floor.addComponent < AABBCollisionComponent>();

            transform.pos = { 0, 480 };
            transform.velocity = { 0, 0 };

            sprite.rectangle.setPosition(transform.pos);
            sprite.rectangle.setSize({ 640, 120 });
            sprite.rectangle.setFillColor(sf::Color::Green);

            aabb.pos = { 0, 0 };
            aabb.size = { 640, 120 };
            floor.addComponent<NameComponent>().name = "Floor";

        }

        {
            Entity wall = Entity::createEntity();
            auto& transform = wall.addComponent<TransformComponent>();
            auto& sprite = wall.addComponent<SpriteComponent>();
            auto& aabb = wall.addComponent < AABBCollisionComponent>();

            transform.pos = { 500, 416 };
            transform.velocity = { 0, 0 };

            sprite.rectangle.setPosition(transform.pos);
            sprite.rectangle.setSize({ 64, 64 });
            sprite.rectangle.setFillColor(sf::Color::Blue);

            aabb.pos = { 0, 0 };
            aabb.size = { 64, 64 };

            wall.addComponent<NameComponent>().name = "Wall";
        }




    }
    void save() const noexcept override
    {
    }
};
REGISTER_SCENE(SylvanScene);

int main(int argc, char** argv) {
    const auto path = std::filesystem::current_path() / std::filesystem::path("../../../../testproject") / "project.json";

    maple_main(path); //bad and non-portable.
}