#pragma once
#include <cstdint>
#include <unordered_set>
#include <unordered_map>
#include "EntityManager.hpp"
#include "MapleComponents.hpp"
#include "MapleContext.hpp"

class Entity {
public:

	Entity() {
		assert(false);
		//bad
	}
	static Entity createEntity() {
		const uint64_t id = context.entityManager->create();
		
		return Entity(id);
	}

	[[nodiscard]] uint64_t getID() const noexcept {
		return id;
	}

	template <typename Component>
	Component& addComponent() {
		assert(!context.entityManager->hasComponent<Component>(id));

		return context.entityManager->addComponent<Component>(id);
	}

	template <typename Component>
	Component& getComponent() {
		assert(context.entityManager->hasComponent<Component>(id));

		return context.entityManager->getComponent<Component>(id);
	}

	template <typename Component>
	bool hasComponent() const noexcept {
		return context.entityManager->hasComponent<Component>(id);
	}

	template <typename... Components>
	bool hasComponents() const noexcept {
		return (context.entityManager->hasComponent<Components>() && ...);
	}

	static MapleServices context;
private:
	Entity(uint64_t id) : id(id) {};

	static Entity createDummyEntity() {
		return Entity(UINT64_MAX); //temp;
	}

	uint64_t id;

	friend class Script;
	friend class EntityManager;
};

inline MapleServices Entity::context = MapleServices();