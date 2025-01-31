#include "AssetRegistry.hpp"

void AssetRegistry::insert(const std::string& name, const AssetProperties& properties, const uint64_t uuid) {
	assert(!nameToUUID.contains(name));
	assert(!UUIDtoName.contains(uuid));

	assert(properties.uuid == uuid);

	UUIDtoName[uuid] = name;
	nameToUUID[name] = properties;
}

bool AssetRegistry::exists(const std::string& name) const noexcept {
	if (!nameToUUID.contains(name)) return false;

	const auto& uuid = nameToUUID.at(name).uuid;

	return UUIDtoName.contains(uuid);
}

bool AssetRegistry::exists(const uint64_t uuid) const noexcept {
	if (!UUIDtoName.contains(uuid)) return false;

	if (!nameToUUID.contains(UUIDtoName.at(uuid))) return false;

	return true;
}

AssetProperties AssetRegistry::getProperties(const uint64_t uuid) const noexcept {
	assert(exists(uuid));

	return nameToUUID.at(UUIDtoName.at(uuid));
}

AssetProperties AssetRegistry::getProperties(const std::string& name) const noexcept {
	assert(exists(name));

	return nameToUUID.at(name);
}

std::string& AssetRegistry::getName(const uint64_t uuid) {
	assert(exists(uuid));

	return UUIDtoName.at(uuid);
}

void AssetRegistry::LoadRegistry(const nlohmann::json& assetData) {
#pragma warning "we do need to save a null thing later"
	if (assetData["assets"].empty()) return;

	for (const auto& jsonObj : assetData["assets"]) {
		const std::string name = jsonObj["name"];
		const AssetProperties properties = AssetProperties::fromJson(jsonObj);

		nameToUUID.insert({ name, properties });
		UUIDtoName.insert({ properties.uuid, name });
	}
}


[[nodiscard]] nlohmann::json AssetRegistry::SaveRegistry() const noexcept {
	nlohmann::json json;
	assert(nameToUUID.size() == UUIDtoName.size());

	if (nameToUUID.empty()) {
		json["assets"] = {};
		return json;
	}


	for (const auto& [name, properties] : nameToUUID) {
		nlohmann::json j = properties.toJson();
		j["name"] = name;
		json["assets"].push_back(j);

	}
	return json;
}

const std::unordered_map<std::string, AssetProperties>& AssetRegistry::getAllAssets() const noexcept {
	return nameToUUID;
}