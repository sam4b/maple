#include "maple.hpp"

int maple_main(const std::filesystem::path& path) {
    const auto project = OpenProject(path);
    runProject(project);
    return 0;
}

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


MapleProject OpenProject(const std::filesystem::path& path) {
    assert(std::filesystem::exists(path));

    std::ifstream projectFile(path);

    const nlohmann::json json = nlohmann::json::parse(projectFile);

    MapleProject project;
    project.root = path.parent_path();
    project.entryPoint = path.parent_path() / json["entryPoint"];
    project.name = json["name"];

    return project;
}