#pragma once
#include <iostream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include "ECSStorage.hpp"
#include "Query.hpp"
#include <functional>
#include "Meta.hpp"
#include <format>
#include "Random.hpp"
#include  <cassert>
#include <nlohmann/json.hpp>
#include "MapleContext.hpp"

class EntityManager {
public:
	template <typename... Components>
	void RunSystem(const float dt, const std::function<void(float, Query<Components...>&, Systems)>& system, Systems systems) {
		Query<Components...> query(entities, getStorage<Components>()...);
		system(dt, query, systems);
		query.PushChangesToOriginal(getStorage<Components>()...);
	}

	void registerComponent(int id, TypeErasedStorage* data, const std::string& name);

	void loadJSON(const nlohmann::json& json, Systems context);

	nlohmann::json toJSON() const noexcept;

	~EntityManager();

	uint64_t create();
	
	template <typename T>
	bool hasComponent(const uint64_t id) {
		return getStorage<T>().contains(id);
	}

	template <typename T>
	T& addComponent(const uint64_t id) {
		assert(entities.contains(id));
		assert(!hasComponent<T>(id));
		return getStorage<T>().add(id);
	}

	template <typename T>
	void removeComponent(const uint64_t id) {
		assert(entities.contains(id));
		assert(hasComponent<T>(id));

		getStorage<T>().remove(id);
	}

	template <typename T>
	T& getComponent(const uint64_t id) {
		assert(hasComponent<T>(id));
		return getStorage<T>().get(id);
	}

	void destroy(const uint64_t id);

	bool enabled = false;

	const std::unordered_set<uint64_t>& getEntities() const noexcept;
private:




	//Alternative ways of getting the same storage.
	std::unordered_map<uint64_t, TypeErasedStorage*> storage;
	std::unordered_map<std::string, TypeErasedStorage*> nameMap;


	std::unordered_set<uint64_t> entities;

	template <typename T>
	ECSStorage<T>& getStorage() {
		const auto typeID = TypeIdentifier::get<T>();

		assert(storage.contains(typeID));

		StorageWrapper<T>* wrapper = (StorageWrapper<T>*)storage.at(typeID);

		return wrapper->get();
	}
};