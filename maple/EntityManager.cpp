#include "EntityManager.hpp"

void EntityManager::registerComponent(int id, TypeErasedStorage* data, const std::string& name) {
	assert(!this->storage.contains(id));
	assert(!this->nameMap.contains(name));


	std::cout << std::format("Successfully registered component {} with typeid {}.\n", name, id);

	storage[id] = data;
	nameMap[name] = data;
}

void EntityManager::loadJSON(const nlohmann::json& json, Systems context) {
	for (const uint64_t id : json["entities"]) {
		entities.insert(id);
	}

	for (const auto componentMap : json["componentMaps"]) {
		if (!nameMap.contains(componentMap["name"])) {
			std::cout << std::format("When trying to read ComponentMap with name {}, could not find registered. Perhaps component not registered?\n", std::string(componentMap["name"]));
			assert(false);
		}

		TypeErasedStorage* storage = nameMap.at(componentMap["name"]);

		storage->fromJSON(componentMap["instances"], context);
	}

}

nlohmann::json EntityManager::toJSON() const noexcept {
	nlohmann::json out;

	for (const auto id : entities) {
		out["ids"].push_back(id);
	}

	for (const auto& [name, map] : nameMap) {
		nlohmann::json jsonMap;
		jsonMap["name"] = name;
		jsonMap["data"] = map->toJSON();

		out["componentMaps"].push_back(jsonMap);
	}

	return out;
}

EntityManager::	~EntityManager()
{
	for (const auto [id, ptr] : storage)
	{
		delete ptr;
	}

	//Not iterating through nameMap as double free.
}

uint64_t EntityManager::create() {
	const int id = generateUUID();
	entities.insert(id);
	return id;
}


void EntityManager::destroy(const uint64_t id) {
	assert(entities.contains(id));
	entities.erase(id);

	for (auto [_, map] : storage) {
		map->destroyIfContains(id);
	}
}

const std::unordered_set<uint64_t>& EntityManager::getEntities() const noexcept {
	return entities;
}