#pragma once
#include "AssetProperties.hpp"

/*
	This is very over the top and should be heavily optimised
*/
class AssetRegistry {
public:
	void insert(const std::string& name, const AssetProperties& properties, const uint64_t uuid) {
		assert(!nameToUUID.contains(name));
		assert(!UUIDtoName.contains(uuid));

		assert(properties.uuid == uuid);

		UUIDtoName[uuid] = name;
		nameToUUID[name] = properties;
	}
	bool exists(const std::string& name) const noexcept {
		if (!nameToUUID.contains(name)) return false;

		const auto& uuid = nameToUUID.at(name).uuid;

		return UUIDtoName.contains(uuid);
	}
	bool exists(const uint64_t uuid) const noexcept {
		if (!UUIDtoName.contains(uuid)) return false;

		if (!nameToUUID.contains(UUIDtoName.at(uuid))) return false;

		return true;
	}

	AssetProperties getProperties(const uint64_t uuid) const noexcept {
		assert(exists(uuid));

		return nameToUUID.at(UUIDtoName.at(uuid));
	}

	AssetProperties getProperties(const std::string& name) const noexcept {
		assert(exists(name));

		return nameToUUID.at(name);
	}

	std::string& getName(const uint64_t uuid) {
		assert(exists(uuid));

		return UUIDtoName.at(uuid);
	}

	void LoadRegistry(const nlohmann::json& assetData) {
		if (assetData["assets"].empty()) return;

		for (const auto& jsonObj : assetData["assets"]) {
			const std::string name = jsonObj["name"];
			const AssetProperties properties = fromJson(jsonObj);

			nameToUUID.insert({ name, properties });
			UUIDtoName.insert({ properties.uuid, name });
		}
	}


	[[nodiscard]] nlohmann::json SaveRegistry() const noexcept {
		nlohmann::json json;
		for (const auto& [name, properties] : nameToUUID) {
			nlohmann::json j = toJson(properties);
			j["name"] = name;
			json["assets"].push_back(j);

		}
		return json;
	}
private:
	std::unordered_map<std::string, AssetProperties> nameToUUID;
	std::unordered_map<uint64_t, std::string> UUIDtoName;
};

