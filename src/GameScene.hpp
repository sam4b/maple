#pragma once
#include "Scene.hpp"
#include "LinkScript.hpp"

struct Event {
    int event;
    enum class EventType {
        Game_Internal,
        Input
    };
};

struct StardewUI {
    sf::RectangleShape background;
    std::vector<sf::RectangleShape> inventoryItems;
};

struct StardewUISettings {
    int inventoryScreenCapacity;
    sf::Vector2f pos;
    int itemSize;
    int padding;
};


StardewUI create_stardew_ui(const StardewUISettings settings) {
    StardewUI ui;

    const sf::Vector2f backgroundSize = { (float)(settings.inventoryScreenCapacity + 1) * settings.itemSize, (float)settings.itemSize };

    ui.background.setSize(backgroundSize);
    ui.background.setFillColor(sf::Color::Green);
    ui.background.setPosition(settings.pos);

    ui.inventoryItems.resize(settings.inventoryScreenCapacity);

    for (int i = 0; i < ui.inventoryItems.size(); i++) {
        sf::RectangleShape rect;

        const int x = settings.padding + i * (settings.itemSize + settings.padding);
        const int y = settings.padding;
        const int size = 64;

        rect.setPosition({ (float)x, (float(y)) });
        rect.move(settings.pos);
        rect.setSize({ (float)size, (float)size });
        rect.setFillColor(sf::Color::Red);
        ui.inventoryItems[i] = rect;
    }
    return ui;
}


void draw_stardew_ui(RenderTarget& target, const StardewUI* const ui) {
    target.draw(ui->background);
    target.draw(ui->inventoryItems);
}

void update_stardew_ui(StardewUI* ui, sf::Vector2i mousePos, int* selected, bool mousePressed) {
    if (*selected != -1) {
        ui->inventoryItems[*selected].setFillColor(sf::Color::Red);
        // *selected = -1;
    }
    for (int i = 0; i < ui->inventoryItems.size(); i++) {
        if (ui->inventoryItems[i].getGlobalBounds().contains(mousePos.x, mousePos.y) && mousePressed) {
            if (i == *selected) { //Deselect
                *selected = -1;
                ui->inventoryItems[i].setFillColor(sf::Color::Red);
            }
            else { //Select
                *selected = i;
            }
        }
    }

    if (*selected == -1) return;

    ui->inventoryItems[*selected].setFillColor(sf::Color::Blue);
}


class StardewScene : public Scene {
public:
    StardewScene() = default;

    void onSceneCreate(MapleServices view, const nlohmann::json& savedData) {

        const StardewUISettings settings = {
            .inventoryScreenCapacity = 5,
            .pos = { 50, 0.6 * 720 },
            .itemSize = 128,
            .padding = 32,
                };
        ui = create_stardew_ui(settings);
        selectedItem = -1;
        player = Entity::createEntity();
        view.scriptManager->addScript<LinkScript>(player, view);
        
        Entity entity2 = Entity::createEntity();
        view.scriptManager->addScript<NPCScript>(entity2, view);

        const std::string path = "test.json";
        assert(std::filesystem::exists(path));
        std::ifstream file(path);

        assert(file.is_open());
        const nlohmann::json json = nlohmann::json::parse(file);
        map.loadFrom(json);
    }


    void draw(RenderTarget& target, MapleServices view) noexcept override {

        sf::RenderTarget* t = target.get();

       /*
        sf::View view_;
        view_.setCenter(player.getComponent<TransformComponent>().pos);
        view_.setSize({ 640, 480 });
        view_.setViewport({ 0,0,1,1 });
        t->setView(view_);*/
        map.draw(target);
        for (Entity sprite : this->getEntities(view)) {
            if (sprite.hasComponent<SpriteComponent>()) {
                target.draw(sprite.getComponent<SpriteComponent>().rectangle);
            }
        }
        t->setView(t->getDefaultView());
        if (!boxFinished) {
            target.draw(box.rectangle);
            target.draw(box.text);
        }
        else {
            draw_stardew_ui(target, &ui);
        }
        // update_camera();
        // draw_tilemap();
        /// draw_npcs();
       // target.resetView();
       // draw_stardew_ui(target, &ui);
    }

    void update(std::queue<sf::Event> events, const sf::Vector2i mousePos, const float dt, MapleServices& view, const sf::Time time) noexcept override {
        bool mousePressed = false;
        while (!events.empty()) {
            const auto front = events.front();
            events.pop();

            if (front.type == sf::Event::MouseButtonPressed) {
                mousePressed = true;
            }
        }

        update_stardew_ui(&ui, mousePos, &selectedItem, mousePressed);
        update_dialog_box(&box, &boxFinished, time);
        ImGui::Begin("Test");
        ImGui::Text(std::format("{}", selectedItem).c_str());
        ImGui::End();
    }
    sf::Vector2i getDrawSize() const noexcept override {
        return { 1280, 720 };
    }

    void save() const noexcept override {

    }



private:
    StardewUI ui;

    Entity player;

    int selectedItem;
};
REGISTER_SCENE(StardewScene);