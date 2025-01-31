#pragma once
#include "Entity.hpp"
#include "EntityManager.hpp"
#include <cassert>
#include <SFML/Graphics.hpp>
#include "Scripting.hpp"

class ScriptManager {
public:
	template <typename T>
	void addScript(Entity entity, Systems context) {
		static_assert (std::is_base_of_v<Script, T>);

		assert(!scriptMap.contains(entity.getID()));

		scriptMap[entity.getID()] = new T();
		assert(scriptMap[entity.getID()]);

		scriptMap[entity.getID()]->bind(entity);
		scriptMap[entity.getID()]->onAttach(context);
	}

	Script* addScript(Entity entity, Systems context, const std::string& name) {
		assert(factoryMap.contains(name));

		assert(!scriptMap.contains(entity.getID()));

		scriptMap[entity.getID()] = factoryMap.at(name)();
		assert(scriptMap[entity.getID()]);

		scriptMap[entity.getID()]->bind(entity);
		scriptMap[entity.getID()]->onAttach(context);

		return scriptMap[entity.getID()];
	}

	void detachScript(Entity entity, EntityManager& entityManager) {
		assert(scriptMap.contains(entity.getID()));

		assert(scriptMap[entity.getID()]);

		scriptMap[entity.getID()]->onDetach();

		delete scriptMap[entity.getID()];

		scriptMap.erase(entity.getID());
	};

	void sendEvent(sf::Event event, Entity entity) {
		scriptMap.at(entity.getID())->receiveEvent(event);
	}

	bool hasScript(Entity entity) {
		return scriptMap.contains(entity.getID());
	}

	Script* getScript(Entity entity) {
		assert(hasScript(entity));

		return scriptMap.at(entity.getID());
	}

	void RegisterScript(std::string name, std::function<Script* ()> func) {
		assert(!factoryMap.contains(name));

		factoryMap[name] = func;
	}

	[[nodiscard]] const std::unordered_map<std::string, std::function<Script* ()>>& getRegisteredScripts() const noexcept {
		return factoryMap;
	}
private:
	std::unordered_map<uint64_t, Script*> scriptMap;

	//Used for creating script by name
	std::unordered_map<std::string, std::function<Script* ()>> factoryMap;
};