#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.hpp"
#include <queue>
#include <cassert>
#include "MapleComponents.hpp"
#include "AssetManager.hpp"
#include <format>

class Systems;

class Script {
public:
	Entity getTarget() const noexcept {
		return entity;
	}

	void bind(Entity entity) {
		this->entity = entity;
	}

	virtual void onAttach(Systems& manager) = 0;

	virtual void onDetach() = 0;

	virtual void tryInteract(Entity interactor) {
		std::cout << std::format("Entity {} tried to interact with me: Entity {}!\n", interactor.getID(), this->getTarget().getID());
	}

	void receiveEvent(sf::Event event) {
		events.push(event);
	}

	virtual void onUpdate(const float dt, Systems& view, Scene* scene) = 0;
protected:
	[[nodiscard]] bool hasEvent(sf::Event& out) noexcept {
		if (events.empty()) return false;

		out = events.front();
		events.pop();

		return true;
	}
	Entity entity = Entity::createDummyEntity();
private:
	std::queue<sf::Event> events;
};


