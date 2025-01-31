#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.hpp"

class RenderTarget {
public:
    RenderTarget(sf::RenderTarget* target) {
        this->target = target;
    }

    void setView(TransformComponent& transform) {
        target->setView({ transform.pos, { 640, 480 } });
    }

    void resetView() {
        target->setView(target->getDefaultView());
    }
    template <sf::PrimitiveType type>
    void draw(const std::vector<sf::Vertex>& vertices) {
        target->draw(vertices.data(), vertices.size(), type);
    }

    void draw(const std::vector<sf::RectangleShape>& shapes) {
        for (const auto& shape : shapes) {
            target->draw(shape);
        }
    }

    void draw(const sf::RectangleShape& shape) {
        target->draw(shape);
    };

    void draw(const sf::Text& text) {
        target->draw(text);
    }

    [[nodiscard]] sf::Vector2i getSize() const noexcept {
        return { (int)target->getSize().x, (int)target->getSize().y };
    }
    
    sf::RenderTarget* get() {
        return target;
    }

private:
    sf::RenderTarget* target;
};