#include <maple.hpp>
#include <editor.hpp>


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

class SylvanScript : public Script {
    // Inherited via Script
    void onAttach(Systems& manager) override
    {
        const auto texture = manager.assetManager->GetTexture("sylvan_spritesheet_3").value();

        SpriteComponent& sprite = entity.addComponent<SpriteComponent>();
        sprite.rectangle.setTexture(texture.texture);
        sprite.rectangle.setTextureRect(texture.rect);
        sprite.rectangle.setSize({ 66, 66 });
        sprite.rectangle.setPosition(0, 0);

        TransformComponent& trans = entity.addComponent<TransformComponent>();;
        trans.pos = { 0, 0 };
        trans.velocity = { 0, 0 };
    }
    void onDetach() override
    {
    }
    void onUpdate(const float dt, Systems& view, Scene* scene) override
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            if (!entity.hasComponent<AnimationStateComponent>()) {
                AddAnimation("sylvan_walk", entity, view);
            }
        }
    }
};
REGISTER_SCRIPT(SylvanScript);

class TestScene : public Scene {
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

    }
    void save() const noexcept override
    {
    }
};
REGISTER_SCENE(TestScene);


class TestScript : public Script {
    void onAttach(Systems& manager) override
    {
        auto& trans = entity.addComponent<TransformComponent>();
        trans.pos = { 128,128 };
        trans.velocity = { 0,0 };
    }
    void onDetach() override
    {
    }
    void onUpdate(const float dt, Systems& view, Scene* scene) override
    {
        sf::Vector2f vel{ 0,0 };
        constexpr float speed = 100;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            vel.x += speed;
        };
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            vel.x -= speed;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            vel.y -= speed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            vel.y += speed;
        }

        entity.getComponent<TransformComponent>().velocity = vel;


    }
};
REGISTER_SCRIPT(TestScript);

int main(int argc, char** argv) {
    const auto path = std::filesystem::current_path() / std::filesystem::path("../../../../testproject") / "project.json";

    std::string s;

    while (s != "editor" && s != "normal") {
        std::cout << "Open in [editor/normal] mode?: ";
        std::cin >> s;
    }

    if (s == "editor") {
        editor_main(path);
    }
    else {
        maple_main(path); //bad and non-portable.
    }
}