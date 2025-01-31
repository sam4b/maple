#include "EntityManager.hpp"

void EntityManager::registerComponent(int id, std::function<ComponentMap* ()> factory, const std::string& name) {
	assert(!components.contains(id));
	assert(!nameToMap.contains(name));

	components[id] = factory();
	nameToMap[name] = components[id];
}

void EntityManager::loadJSON(const nlohmann::json& json, Systems context) {
	for (const uint64_t id : json["entities"]) {
		entities.insert(id);
	}

	for (const auto componentMap : json["componentMaps"]) {
		if (!nameToMap.contains(componentMap["name"])) {
			std::cout << std::format("When trying to read ComponentMap with name {}, could not find registered. Perhaps component not registered?\n", std::string(componentMap["name"]));
			assert(false);
		}

		ComponentMap* map = nameToMap.at(componentMap["name"]);

		map->fromJSON(componentMap["instances"], context);
	}

}

nlohmann::json EntityManager::toJSON() const noexcept {
	nlohmann::json out;

	for (const auto id : entities) {
		out["ids"].push_back(id);
	}

	for (const auto& [name, map] : nameToMap) {
		nlohmann::json jsonMap;
		jsonMap["name"] = name;
		jsonMap["data"] = map->toJSON();

		out["componentMaps"].push_back(jsonMap);
	}

	return out;
}

EntityManager::~EntityManager() {
	//deal w ptrs
}

uint64_t EntityManager::create() {
	const int id = generateUUID();
	entities.insert(id);
	return id;
}


void EntityManager::destroy(const uint64_t id) {
	assert(entities.contains(id));
	entities.erase(id);

	for (auto [_, map] : components) {
		map->destroyIfContains(id);
	}
}

const std::unordered_set<uint64_t>& EntityManager::getEntities() const noexcept {
	return entities;
}