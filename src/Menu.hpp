#pragma once
#include "Scene.hpp"

//Maple.SwitchScene<GameScene>("newGame.json");
//Maple.CloseApplication();
//Provide through Scene?

typedef int ItemID;
std::vector<ItemID> inventory;

class UIButton {
public:
    void hover() {

    }

    void unhover() {

    }
    
    void leftclick() {

    }

    void rightclick() {

    }
};

class Menu : public Scene {
private:
    struct UILayer {
        std::vector<sf::RectangleShape> buttons;
        enum class OnClick {
            None,
            Exit,
            NewGame,
            LoadGame
        };

        std::vector<OnClick> event;
    };
    std::vector<UILayer> layers;
public:
    Menu() {
        layers.push_back({});
        layers.back().event.push_back(UILayer::OnClick::None);
        sf::RectangleShape r;
        r.setSize({ 128, 128 });
        r.setFillColor(sf::Color::Red);
        r.setPosition(0, 0); 
        layers.back().buttons.push_back(r);
    }

    void onSceneCreate(MapleServices view, const nlohmann::json& json) override {

    }

    void draw(RenderTarget& target, MapleServices view) noexcept override {
        for (const auto layer : std::ranges::views::reverse(layers)) {
            for (const auto& rect : layer.buttons) {
                target.draw(rect);
            }
        }
    }
    void update(std::queue<sf::Event> events, const sf::Vector2i mousePos, const float dt, MapleServices& view, const sf::Time time) noexcept override {
        while (!events.empty()) {
            const auto event = events.front();

            switch (event.type) {
           
            case sf::Event::MouseButtonPressed:
            {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    for (const UILayer& layer : layers) {
                        assert(layer.buttons.size() == layer.event.size());
                        const auto size = layer.buttons.size();
                        for (int i = 0; i < size; i++) {
                            if (layer.buttons[i].getGlobalBounds().contains(mousePos.x, mousePos.y)) {
//                                view.newScene("", SceneIdentifier::Game);
                            }
                        };
                    };
                    //Loop over each UI layer top to bottom
                    //Check if any overlap
                    //If muliple, take overlap that greatest
                    //What if UI overlays anotehr UI layre
                    //Regardless, invoke event.
                 //  assert(false);
                    }
                break;

            }
            case sf::Event::MouseMoved:
            {
             //   assert(false);
                static std::optional<int> selected = {};

                if (selected.has_value()) {
                    layers[0].buttons[selected.value()].setFillColor(sf::Color::Red);
                }

                for (const UILayer& layer : layers) {
                    assert(layer.buttons.size() == layer.event.size());
                    const auto size = layer.buttons.size();
                    for (int i = 0; i < size; i++) {
                        if (layer.buttons[i].getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                            selected = i;

                            layers[0].buttons[selected.value()].setFillColor(sf::Color::Blue);

                        }
                    };
                };
            }
            }
            std::cout << (events.front().type == sf::Event::MouseButtonPressed) << std::endl;
            events.pop();
        }
    }
    sf::Vector2i getDrawSize() const noexcept override {
        return { 0,0 };
    }

    void save() const noexcept override {

    }
private:
};

struct Button {
    sf::RectangleShape shape;
    enum class ButtonType {
        NewGame,
        LoadGame,
        Exitz
    };
};