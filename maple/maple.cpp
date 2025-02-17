#include "maple.hpp"
#include <numeric>
#include <limits>

int maple_main(const std::filesystem::path& path) {
    const auto project = OpenProject(path);
    runProject(project);
    return 0;
}

Maple LoadProject(const MapleProject& project) {
    Maple maple;
    maple.systems.assetManager = new AssetManager(project.root);
    maple.systems.entityManager = new EntityManager();
    maple.systems.scriptManager = new ScriptManager();

    const auto [components, scripts, scenes] = Registry::GetUserContents();

    for (const auto& [name, componentData] : components) {
        maple.systems.entityManager->registerComponent(componentData.typeID, componentData.data, name);;
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

    maple.systems.assetManager->LoadAllAssetsInRegistry(project.root);
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
    Adapted from: https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/
*/


std::optional<sf::Vector2f> updateVelocityIfCollided(const AABBCollisionComponent& box1, const AABBCollisionComponent& box2,
    const TransformComponent& t1, const TransformComponent& t2) {

    sf::Vector2f closestEdge;
    sf::Vector2f distToFarSide;



    if (t1.velocity.x > 0.0f)
    {
        closestEdge.x = t2.pos.x - (t1.pos.x + box1.size.x);
        distToFarSide.x = (t2.pos.x + box2.size.x) - t1.pos.x;
    }
    else
    {
        closestEdge.x = (t2.pos.x + box2.size.x) - t1.pos.x;
        distToFarSide.x = t2.pos.x - (t1.pos.x + box1.size.x);
    }

    if (t1.velocity.y > 0.0f)
    {
        closestEdge.y = t2.pos.y - (t1.pos.y + box1.size.y);
        distToFarSide.y = (t2.pos.y + box2.size.y) - t1.pos.y;
    }
    else
    {
        closestEdge.y = (t2.pos.y + box2.size.y) - t1.pos.y;
        distToFarSide.y = t2.pos.y - (t1.pos.y + box1.size.y);
    }

    sf::Vector2f entryTimeVector;
    sf::Vector2f exitTimeVector;

    //calc

    if (t1.velocity.x == 0.0f)
    {
        entryTimeVector.x = -std::numeric_limits<float>::infinity();
        exitTimeVector.x = std::numeric_limits<float>::infinity();
    }
    else
    {
        entryTimeVector.x = closestEdge.x / t1.velocity.x;
        exitTimeVector.x = distToFarSide.x / t1.velocity.x;
    }

    if (t1.velocity.y == 0.0f)
    {
        entryTimeVector.y = -std::numeric_limits<float>::infinity();
        exitTimeVector.y = std::numeric_limits<float>::infinity();
    }
    else
    {
        entryTimeVector.y = closestEdge.y / t1.velocity.y;
        exitTimeVector.y = distToFarSide.y / t1.velocity.y;
    }

    const float entryTime = std::max(entryTimeVector.x, entryTimeVector.y);
    const float exitTime = std::min(exitTimeVector.x, exitTimeVector.y);

    const bool noCollision = entryTime > exitTime 
        || entryTimeVector.x < 0.0f && entryTimeVector.y < 0.0f 
        || entryTimeVector.x > 1.0f 
        || entryTimeVector.y > 1.0f;



    if (noCollision) {
         return std::nullopt;
    }

    std::cout << "Collision happened!\n";

    //Calculate normal
    sf::Vector2f normal;

    if (entryTimeVector.x > entryTimeVector.y) {
        normal = ((closestEdge.x >= 0.0f) ? -1.0f : 1.0f) * sf::Vector2f{ 1.0f, 0.0f };
    }
    else {
        normal = ((closestEdge.y >= 0.0f) ? -1.0f : 1.0f) * sf::Vector2f{ 0.0f, 1.0f };
    }

    const float collisionTime = entryTime;
    const float remainingTime = 1.0f - collisionTime;

    const float dot = ((t1.velocity.x * normal.y) + (t1.velocity.y * normal.x)) * remainingTime;


    const sf::Vector2f resolve = dot * sf::Vector2f{ normal.y, normal.x };

    return resolve;
}

bool AABBCheck(const AABBCollisionComponent& box1, const AABBCollisionComponent& box2) {
    return !(box1.pos.x + box1.size.x < box2.pos.x || box1.pos.x > box2.pos.x + box2.size.x || box1.pos.y + box1.size.y < box2.pos.y || box1.pos.y > box2.pos.y + box2.size.y);
}


std::optional<sf::Vector2f> collision(const AABBCollisionComponent& box1, const AABBCollisionComponent& box2,
    const TransformComponent& t1, const TransformComponent& t2) {


    //This is minkowski?

    //Not colliding yet, is this in our broadphase box?


    AABBCollisionComponent broadphase;
    broadphase.pos.x = t1.velocity.x > 0 ? box1.pos.x : box1.pos.x + t1.velocity.x;
    broadphase.pos.y = t1.velocity.y > 0 ? box1.pos.y : box1.pos.y + t1.velocity.y;
    broadphase.size.x = t1.velocity.x > 0 ? t1.velocity.x + box1.size.x : box1.size.x - t1.velocity.x;
    broadphase.size.y = t1.velocity.y > 0 ? t1.velocity.y + box1.size.y : box1.size.y - t1.velocity.y;


    if (!AABBCheck(broadphase, box2)) {
        return std::nullopt;
    }

    //We could collide, run swept AABB.

    const auto value = updateVelocityIfCollided(box1, box2, t1, t2);

    if (!value.has_value()) {
        return std::nullopt;
    }
    return value;
}




/*
    Somehow this has gained responsibility for drawing...
*/
void step(Scene* scene, sf::Time time, RenderTarget& target, std::queue<sf::Event> events, Systems context, sf::Vector2i mousePos) {
    const float dt = time.asSeconds();
    std::queue<uint64_t> toKill; //Move to bump allocator.

    scene->update(events, mousePos, time.asSeconds(), context, time);

    for (Script* script : scene->getEntities(context)
        | std::ranges::views::filter([&](Entity e) -> bool { return context.scriptManager->hasScript(e); })
        | std::ranges::views::transform([&](Entity e) -> Script* { return context.scriptManager->getScript(e); })) {
        script->onUpdate(time.asSeconds(), context, scene);
    }

    /*
        Animation system
    */

    const std::function<void(const float, Query<AnimationStateComponent, SpriteComponent, TransformComponent>&, Systems)> func = 
        [](const float dt, Query<AnimationStateComponent, SpriteComponent, TransformComponent>& query, Systems context) -> void
	{
            for (auto& [state, sprite, transform] : query.iterate()) {
                const uint64_t subtract = dt * 100;

                const auto& animation = context.assetManager->GetAnimation(state.animationID);

                state.lastUpdate -= subtract;

                if (state.lastUpdate <= 0) {
                    state.offset++;
                    state.lastUpdate = animation.frameTime;
                    if (state.offset == animation.ids.size()) {
                        state.offset = 0;
                    }
                }


                if (state.lastUpdate <= 0) {
                    state.lastUpdate = animation.frameTime;
                    state.offset++;
                    if (state.offset == animation.ids.size()) state.offset = 0;
                }

                const auto tex = context.assetManager->GetTexture(animation.ids[state.offset]).value();

                sprite.rectangle.setTextureRect(tex.rect);
                sprite.rectangle.setTexture(tex.texture);
            }
	};

    context.entityManager->RunSystem<AnimationStateComponent, SpriteComponent, TransformComponent>(dt, func, context);
    


    for (Entity entity : scene->getEntities(context) | std::ranges::views::filter([&](Entity e) -> bool { return e.hasComponent<AnimationStateComponent>(); })) {
        assert(entity.hasComponent<SpriteComponent>());
        assert(entity.hasComponent<TransformComponent>());

        const uint64_t subtract = time.asSeconds() * 100;

        auto& component = entity.getComponent<AnimationStateComponent>();

        const auto& animation = context.assetManager->GetAnimation(component.animationID);

        component.lastUpdate -= time.asSeconds() * 100;

        if (component.lastUpdate <= 0) {
            component.offset++;
            component.lastUpdate = animation.frameTime;
            if (component.offset == animation.ids.size()) {
                component.offset = 0;
            }
        }


        if (component.lastUpdate <= 0) {
            component.lastUpdate = animation.frameTime;
            component.offset++;
            if (component.offset == animation.ids.size()) component.offset = 0;
        } 

        const auto tex = context.assetManager->GetTexture(animation.ids[component.offset]).value();

        auto& sprite = entity.getComponent<SpriteComponent>();

        sprite.rectangle.setTextureRect(tex.rect);
        sprite.rectangle.setTexture(tex.texture);
        //update frame etc

        //if looping, continue
        //else revert to previous state?
        
    }


    static bool g_active = false;
    static bool c_active = false;
    static bool draw_colliders = false;
    ImGui::Begin("Dev Tools");
    ImGui::Checkbox("Draw colliders", &draw_colliders);
    ImGui::Checkbox("Gravity", &g_active);
    ImGui::Checkbox("Collision", &c_active);
    ImGui::End();


    if (c_active) {
        context.entityManager->enabled = true;
    }
    /*
    Gravity system
    */


    if (g_active) {
        for (Entity entity : scene->getEntities(context)
            | std::ranges::views::filter([&](Entity e) -> bool { return e.hasComponent<TransformComponent>(); })) {

            auto& transform = entity.getComponent<TransformComponent>();
            if (entity.hasComponent<Physics2DComponent>()) {
                const auto& physics = entity.getComponent<Physics2DComponent>();
                transform.velocity += {0, physics.mass * 9.81f};
            }
        }

    }

    for (Entity entity : scene->getEntities(context)
        | std::ranges::views::filter([&](Entity e) -> bool {
            return e.hasComponent<TransformComponent>();
            })) {
        entity.getComponent<TransformComponent>().velocity *= dt;
    }

    /* Collision System */

    if (c_active) {

        context.entityManager->RunSystem<TransformComponent, AABBCollisionComponent>(dt, [](const float dt, Query<TransformComponent, AABBCollisionComponent>& query, Systems systems) -> void {
            for (int i = 0; i < query.iterate().size(); i++) {
                auto& [t1, b1] = query.iterate().at(i);
                if (t1.velocity == sf::Vector2f{ 0, 0 }) continue;
                for (int j = 0; j < query.iterate().size(); j++) {
                    if (i == j) continue;
                    const auto& [t2, b2] = query.iterate().at(j);

                    const std::optional<sf::Vector2f> resolveVelocity = collision(b1, b2, t1, t2);

                    if (resolveVelocity.has_value()) {
                        t1.velocity = resolveVelocity.value();
                    }
                }
            }

            }, context);
       
       }

    /* Transform system */
    for (Entity entity : scene->getEntities(context)
        | std::ranges::views::filter([&](Entity e) -> bool {
            return e.hasComponent<TransformComponent>();
            })
        ) {

        auto& transform = entity.getComponent<TransformComponent>();
        ImGui::Begin("Transform");
        ImGui::Text(std::format("name: {}, pos: {}, {}, vel: {}, {}", (entity.hasComponent<NameComponent>()) ? entity.getComponent<NameComponent>().name : std::to_string(entity.getID()), transform.pos.x, transform.pos.y, transform.velocity.x, transform.velocity.y).c_str());
        ImGui::End();
        transform.pos += transform.velocity;
        transform.velocity = { 0, 0 };

        if (entity.hasComponent<AABBCollisionComponent>()) {
            auto& aabb = entity.getComponent<AABBCollisionComponent>();

            aabb.pos = transform.pos;
        }

        if (entity.hasComponent<SpriteComponent>()) {
            auto& sprite = entity.getComponent<SpriteComponent>();

            sprite.rectangle.setPosition(transform.pos);
        }
       
    }

    /* Drawing system */

    for (Entity entity : scene->getEntities(context) | std::ranges::views::filter([&](Entity e) -> bool { return e.hasComponent<SpriteComponent>(); })) {
        target.draw(entity.getComponent<SpriteComponent>().rectangle);
    }

    if (draw_colliders) {
        ImGui::Begin("Collider Info");
        for (Entity entity : scene->getEntities(context) | std::ranges::views::filter([&](Entity e) -> bool { return e.hasComponent<AABBCollisionComponent>(); })) {
            const auto& aabb = entity.getComponent<AABBCollisionComponent>();
            
            sf::RectangleShape shape;
            
            ImGui::Text(std::format("name: {}, pos: {}, {}, size: {}, {}", (entity.hasComponent<NameComponent>()) ? entity.getComponent<NameComponent>().name : std::to_string(entity.getID()), aabb.pos.x, aabb.pos.y, aabb.size.x, aabb.size.y).c_str());
            shape.setPosition(aabb.pos);
            shape.setSize(aabb.size);
            shape.setOutlineColor(sf::Color::Red);
            shape.setOutlineThickness(1.0f);
            shape.setFillColor(sf::Color(0, 0, 0, 0));
            target.draw(shape);
            
        }
        ImGui::End();
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