#pragma once
#include "AssetProperties.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>


/*
	This is very over the top and should be heavily optimised
*/
class AssetRegistry {
public:
	void insert(const std::string& name, const AssetProperties& properties, const uint64_t uuid);

	bool exists(const std::string& name) const noexcept;

	bool exists(const uint64_t uuid) const noexcept;

	AssetProperties getProperties(const uint64_t uuid) const noexcept;

	AssetProperties getProperties(const std::string& name) const noexcept;

	std::string& getName(const uint64_t uuid);

	void LoadRegistry(const nlohmann::json& assetData);

	[[nodiscard]] nlohmann::json SaveRegistry() const noexcept;

	const std::unordered_map<std::string, AssetProperties>& getAllAssets() const noexcept;
private:
	std::unordered_map<std::string, AssetProperties> nameToUUID;
	std::unordered_map<uint64_t, std::string> UUIDtoName;
};

