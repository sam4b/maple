#pragma once
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include "ComponentMap.hpp"
#include <functional>
#include "Meta.hpp"
#include <format>
#include "Random.hpp"

class EntityManager {
public:
	void registerComponent(int id, std::function<ComponentMap* ()> factory, const std::string& name) {
		assert(!components.contains(id));
		assert(!nameToMap.contains(name));

		components[id] = factory();
		nameToMap[name] = components[id];
	}

	void loadJSON(const nlohmann::json& json, MapleServices context) {
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

	nlohmann::json toJSON() const noexcept {
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

	~EntityManager() {
		//deal w ptrs
	}

	uint64_t create() {
		const int id = generateUUID();
		entities.insert(id);
		return id;
	}
	
	template <typename T>
	bool hasComponent(const uint64_t id) {
		const auto& map = this->getMap<T>();

		return map.contains(id);
	}

	template <typename T>
	T& addComponent(const uint64_t id) {
		assert(entities.contains(id));
		assert(!hasComponent<T>(id));

		std::unordered_map<uint64_t, T>& map = getMap<T>();
		map.insert({id, T() });
		return map.at(id);
	}

	template <typename T>
	void removeComponent(const uint64_t id) {
		assert(entities.contains(id));
		assert(hasComponent<T>(id));

		std::unordered_map<uint64_t, T>& map = getMap<T>();
		map.erase(id);
	}

	template <typename T>
	T& getComponent(const uint64_t id) {
		assert(hasComponent<T>(id));
		return getMap<T>().at(id);
	}

	void destroy(const uint64_t id) {
		assert(entities.contains(id));
		entities.erase(id);

		for (auto [_, map] : components) {
			map->destroyIfContains(id);
		}
	}

	std::unordered_set<uint64_t> getEntities() {
		return entities;
	}
private:
	std::unordered_set<uint64_t> entities;

	std::unordered_map<int, ComponentMap*> components;
	std::unordered_map<std::string, ComponentMap*> nameToMap;

	template <typename T>
	std::unordered_map<uint64_t, T>& getMap() {
		const auto typeID = TypeIdentifier::get<T>();

		assert(components.contains(typeID));

		return *(std::unordered_map<uint64_t, T>*)components.at(typeID)->getMap();
	}
};