#include <maple.hpp>
#include "AssetGUI.hpp"

void runEditorMode(const MapleProject& project) {

    Maple maple = LoadProject(project);
    maple.systems.assetManager->LoadAllAssetsInRegistry(project.root);

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

        Assets(*maple.systems.assetManager);
        AssetWindow(*maple.systems.assetManager, maple.systems.assetManager->GetRegistry());
        window.clear();
        ImGui::SFML::Render(window);
        window.display();

    }


    std::cout << "Saving asset registry.\n";
    std::ofstream f(project.root / "assetregistry.json");
    f << maple.systems.assetManager->SaveRegistry();
    std::cout << "Asset registry saved successfuly.\n";


}


int editor_main(const std::filesystem::path& path) {
    const auto project = OpenProject(path);
    runEditorMode(project);
    return 0;
}