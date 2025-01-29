#include "EditorScene.hpp"
#include "GameScene.hpp"
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <cassert>
#include <queue>
#include <imgui.h>
#include <imgui-SFML.h>
#include <nlohmann/json.hpp>
#include <cassert>
#include <iostream>
#include <string>
#include <fstream>
#include "Scripting.hpp"
#include "EntityManager.hpp"
#include "Menu.hpp"
#include "Meta.hpp"
#include "SceneParser.hpp"
#include <magic_enum.hpp>
#include "AssetGUI.hpp"

struct MapleProject {
    std::filesystem::path root;
    std::string name;
    std::filesystem::path entryPoint;
};

struct Maple {
    Systems systems;
    UserContents userConents;
    Scene* currentScene;
};

Maple LoadProject(const MapleProject& project) {
    Maple maple;
    maple.systems.assetManager = new AssetManager();
    maple.systems.entityManager = new EntityManager();
    maple.systems.scriptManager = new ScriptManager();
    
    const auto [components, scripts, scenes] = Registry::GetUserContents();

    for (const auto& [name, componentData] : components) {
        maple.systems.entityManager->registerComponent(componentData.typeID, componentData.storageFactory, name);
        std::cout << std::format("Registered component {}.\n", name);
    }

    for (const auto& [name, creator] : scripts) {
        maple.systems.scriptManager->RegisterScript(name, creator);
        std::cout << std::format("Registered script {}.\n", name);
    }

    for (const auto& [name, scene] : scenes) {
        std::cout << std::format("Registered scene {}.\n", name);
    }

    Entity::context = maple.systems;



    std::filesystem::path registry(project.root / "assetregistry.json");
    if (std::filesystem::exists(registry)) { //No issue if it's the first time, we can create on shutdown.
        std::ifstream file(registry);
        assert(file.is_open());
        const nlohmann::json assetRegistry = nlohmann::json::parse(file);
        maple.systems.assetManager->LoadRegistry(assetRegistry);
    };



    const nlohmann::json json = [&]() -> nlohmann::json {
        std::cout << std::filesystem::current_path() << "\n";
        std::filesystem::path p = project.entryPoint;
        assert(std::filesystem::exists(p));
        std::ifstream f(p);
        assert(f.is_open());
        return nlohmann::json::parse(f);
        }();

    maple.currentScene = ParseScene(json, scenes, maple.systems, project.root);

    return maple;
}

std::queue<sf::Event> readEvents(sf::RenderWindow& window, bool& shouldClose) {
    sf::Event e;
    std::queue<sf::Event> events;
    ImGuiIO& io = ImGui::GetIO();
    while (window.pollEvent(e)) {
        ImGui::SFML::ProcessEvent(e);
        switch (e.type) {
            using enum sf::Event::EventType;

        case MouseButtonPressed:
            if (!io.WantCaptureMouse) {
                events.push(e);
            }
            break;
        case KeyPressed:
            if (!io.WantCaptureKeyboard) {
                if (e.key.code == sf::Keyboard::Escape) {
                    shouldClose = true;
                }
                events.push(e);
            }

            break;
        case MouseButtonReleased:
            if (!io.WantCaptureMouse) {
                events.push(e);
            }
            break;
        case MouseMoved:
            if (!io.WantCaptureMouse) {
                events.push(e);
            }
            break;
        case KeyReleased:
            if (!io.WantCaptureKeyboard) {
                events.push(e);
            }
            break;
        case Closed:
            shouldClose = true;
            break;
        default:
            break;
        }


    }
    return events;
}

/*
    Somehow this has gained responsibility for drawing...
*/
void step(Scene* scene, sf::Time time, RenderTarget& target, std::queue<sf::Event> events, Systems context, sf::Vector2i mousePos) {
    std::queue<uint64_t> toKill; //Move to bump allocator.

    scene->update(events, mousePos, time.asSeconds(), context, time);

    for (Script* script : scene->getEntities(context)
        | std::ranges::views::filter([&](Entity e) -> bool { return context.scriptManager->hasScript(e); })
        | std::ranges::views::transform([&](Entity e) -> Script* { return context.scriptManager->getScript(e); })) {
        script->onUpdate(time.asSeconds(), context, scene);
    }

    for (Entity entity : scene->getEntities(context)
        | std::ranges::views::filter([&](Entity e) -> bool { return e.hasComponent<TransformComponent>(); })) {
        auto& transform = entity.getComponent<TransformComponent>();
        transform.pos += transform.velocity * time.asSeconds();
        transform.velocity = { 0, 0 };

        if (entity.hasComponent<SpriteComponent>()) {
            auto& sprite = entity.getComponent<SpriteComponent>();
            sprite.rectangle.setPosition(transform.pos);
        }

        if (entity.hasComponent<AABBCollisionComponent>()) {
            auto& aabb = entity.getComponent<AABBCollisionComponent>();
            aabb.pos = transform.pos;
        }
    }

    for (Entity entity : scene->getEntities(context) | std::ranges::views::filter([&](Entity e) -> bool { return e.hasComponent<SpriteComponent>(); })) {
        target.draw(entity.getComponent<SpriteComponent>().rectangle);
    }

    //Collision resolution?

}



void runProject(const MapleProject& project) {
    Maple maple = LoadProject(project);

    bool shouldClose = false;
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Editor");

    assert(ImGui::SFML::Init(window));

    RenderTarget target(&window);

    sf::Clock clock;

    bool playing = true;

    while (!shouldClose) {
        const auto time = clock.restart();
        auto events = readEvents(window, shouldClose);



        ImGui::SFML::Update(window, time);
        const auto windowMousePos = sf::Mouse::getPosition(window);
        const auto sceneMousePos = getRelativeToLatestImGuiTopLeft(windowMousePos);

        window.clear();
        step(maple.currentScene, time, target, events, maple.systems, sceneMousePos);
        maple.currentScene->draw(target, maple.systems);
        ImGui::SFML::Render(window);
        window.display();

    }
}

void log(const std::string& string) {
    std::cout << string << "\n";
}

void LoadScene(Maple& maple, std::filesystem::path path) {
   // ParseScene();
}



void runEditorMode(const MapleProject& project) {

    Maple maple = LoadProject(project);

    bool shouldClose = false;
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Editor");

    assert(ImGui::SFML::Init(window));

    sf::RenderTexture texture;

    const sf::Vector2f drawSize = { 1280, 720 };
    assert(texture.create(drawSize.x, drawSize.y));

    RenderTarget target(&texture);

    sf::Clock clock;

    bool playing = true;

    bool tilesetOpen = false;
    int tilesetID = -1;

    TilesetWindow tilesetWindow;

    std::queue<uint64_t> toKill;

    std::vector<ComponentData> registeredComponents;

    //fill!
    
    while (!shouldClose) {
        const auto time = clock.restart();
        auto events = readEvents(window, shouldClose);



        ImGui::SFML::Update(window, time);

        ImGui::Begin("Entities");

        for (auto id : maple.systems.entityManager->getEntities()) {
            ImGui::PushID(id);

  
            if (ImGui::CollapsingHeader(maple.systems.entityManager->hasComponent<NameComponent>(id) ?
                maple.systems.entityManager->getComponent<NameComponent>(id).name.c_str() :
                std::format("Entity ID: {}", id).c_str())) {

                if (ImGui::Button("Kill")) {
                    toKill.push(id);
                }

                int selectedItem = -1;

        

                if (ImGui::BeginCombo("Add components", "test")) { //If no registered components, this is a crash.
                    ImGui::Selectable("items[n]", 1);
                    ImGui::EndCombo();
                }

                for (auto& [name, componentData] : maple.userConents.mapFactory) {
                    componentData.editorShowable(name, *maple.systems.entityManager, id);
                }

            }
            


            ImGui::PopID();
        }
        ImGui::End();


        constexpr int NO_BORDER = 0;
        ImGui::Begin("Scene");
        if (ImGui::Button((playing) ? "Pause" : "Play")) {
            playing = !playing;
        }
        const auto windowMousePos = sf::Mouse::getPosition(window);
        const auto sceneMousePos = getRelativeToLatestImGuiTopLeft(windowMousePos);
        if (ImGui::ImageButton(texture, drawSize, NO_BORDER)) {
            if (playing) {
                sf::Event e;

                e.type = sf::Event::MouseButtonPressed;
                e.mouseButton.button = sf::Mouse::Button::Left;
                e.mouseButton.x = sceneMousePos.x;
                e.mouseButton.y = sceneMousePos.y;
                events.push(e);
            }
            //Get mouse pos relative to scene.
            //Send event to scene.
        }

        if (playing) {
           // log(std::format("Scene mouse pos: {}, {}.", sceneMousePos.x, sceneMousePos.y));
            texture.clear();

            step(maple.currentScene, time, target, events, maple.systems, sceneMousePos);

            maple.currentScene->draw(target, maple.systems);
            texture.display();

            while (!toKill.empty()) {
                const auto id = toKill.front();
                toKill.pop();
                if (maple.systems.scriptManager->hasScript(*(Entity*)&id)) {
                    maple.systems.scriptManager->detachScript(*(Entity*)&id, *maple.systems.entityManager);
                }

                maple.systems.entityManager->destroy(id);
            }
        }
        else {
            clock.restart();
        }

        ImGui::End();


     //   maple.systems.assetManager->ImGuiDisplay();

        if (tilesetOpen) {
            tilesetWindow.update(tilesetOpen);
        }

        AssetWindow(*maple.systems.assetManager, maple.systems.assetManager->GetRegistry(), project.root);
        window.clear();
        ImGui::SFML::Render(window);
        window.display();

    }


    std::cout << "Saving asset registry.\n";
    std::ofstream f(project.root / "assetregistry.json");
    f << maple.systems.assetManager->SaveRegistry();
    std::cout << "Asset registry saved successfuly.\n";


}

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
        Entity entity = Entity::createEntity();
        SpriteComponent& sprite = entity.addComponent<SpriteComponent>();
        const Texture texture = view.assetManager->GetTexture("spritesheet_1000").value();
        sprite.rectangle.setTexture(texture.texture);
        sprite.rectangle.setTextureRect(texture.rect);

        sprite.rectangle.setSize({ 128, 128 });
        sprite.rectangle.setPosition(0, 0);


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
    const std::filesystem::path path = std::filesystem::current_path() / "../../../../testproject/project.json"; //Eventually move to argv.

    assert(std::filesystem::exists(path));
    
    bool editor = true;

    std::ifstream projectFile(path);

    const nlohmann::json json = nlohmann::json::parse(projectFile);

    MapleProject project;
    project.root = path.parent_path();
    project.entryPoint = path.parent_path() / json["entryPoint"];
    project.name = json["name"];



    if (!editor) {
        runProject(project);
    }
    else {
        runEditorMode(project);
    }
    return 0;
}