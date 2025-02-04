#pragma once
#include <iostream>
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
	void registerComponent(int id, std::function<ComponentMap* ()> factory, const std::string& name);

	void loadJSON(const nlohmann::json& json, Systems context);

	nlohmann::json toJSON() const noexcept;

	~EntityManager();

	uint64_t create();
	
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

	void destroy(const uint64_t id);

	bool enabled = false;

	const std::unordered_set<uint64_t>& getEntities() const noexcept;
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