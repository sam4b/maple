#include <maple.hpp>
#include <editor.hpp>

class TestScene : public Scene {
    void draw(RenderTarget& target, Systems view) noexcept override
    {
    }
    void update(std::queue<sf::Event> events, const sf::Vector2i mousePos, const float dt, Systems& view, const sf::Time time) noexcept override
    {
        ImGui::Begin("A");
        if (ImGui::Button("Increment")) {
            static int i = 0;
            for (const auto entity : view.entityManager->getEntities()) {
                if (view.entityManager->hasComponent<SpriteComponent>(entity)) {
                    const Texture t = view.assetManager->GetTexture(std::format("spritesheet_{}", i)).value();
                    auto& sprite = view.entityManager->getComponent<SpriteComponent>(entity);
                    sprite.rectangle.setTextureRect(t.rect);
                    i++;
                }
            }
        }
        ImGui::End();


    }
    sf::Vector2i getDrawSize() const noexcept override
    {
        return sf::Vector2i();
    }
    void onSceneCreate(Systems view, const nlohmann::json& savedData) override
    {
        /*
         Entity entity = Entity::createEntity();
         SpriteComponent& sprite = entity.addComponent<SpriteComponent>();
         const Texture texture = view.assetManager->GetTexture("spritesheet_1000").value();
         sprite.rectangle.setTexture(texture.texture);
         sprite.rectangle.setTextureRect(texture.rect);

         sprite.rectangle.setSize({ 128, 128 });
         sprite.rectangle.setPosition(0, 0);*/


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