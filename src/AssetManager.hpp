#pragma once
#include <SFML/Graphics.hpp>
#include <variant>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <format>
#include <imgui.h>
#include <magic_enum.hpp>
#include "imgui_stdlib.h"
#include "vendor/imgui-filebrowser/imfilebrowser.h"
#include "Utils.hpp"
#include "AssetRegistry.hpp"

struct Texture {
	sf::Texture* texture;
	sf::IntRect rect;
};
/*
	Consider ref-counted handle system. Also support for other types of assets + subtextures (tilemaps?).
*/
class AssetManager {
public:
	void LoadRegistry(const nlohmann::json& json) noexcept {
		registry.LoadRegistry(json);
	}

	/*Need to find a nice way to supply this to only who I want (friend class?)*/
	AssetRegistry& GetRegistry() noexcept {
		return registry;
	}
	
	nlohmann::json SaveRegistry() const noexcept {
		return registry.SaveRegistry();
	}
	/*
		Returns an texture if the texture has been imported into Maple.
		This method attempts to load the texture from disk if it has been imported but is not in memory.
	*/
	std::optional<Texture> GetTexture(const uint64_t id) {
		if (subTextures.contains(id)) {
			const TextureData& textureData = subTextures.at(id);

			assert(textures.contains(textureData.parent)); //parent must be loaded

			assert(textureData.texture.texture); //Non-null.

			return { textureData.texture };
		}

		const AssetProperties properties = registry.getProperties(id);

		assert(properties.type == AssetProperties::Type::SubTexture);

		const uint64_t parentID = properties.extraneous.subTextureData.parentUUID;

		if (!textures.contains(parentID)) {
			const bool success = textures[parentID].loadFromFile(std::format("{}.png", parentID));

			assert(success);
		}

		TextureData data;
		data.parent = parentID;
		data.texture.rect = properties.extraneous.subTextureData.rect;
		data.texture.texture = &textures.at(parentID);

		subTextures[id] = data;

		return subTextures[id].texture;
	}

	std::optional<Texture> GetTexture(const std::string& path) {
		assert(registry.exists(path));
		return GetTexture(registry.getProperties(path).uuid);
	}

	//Must be called after loading the registry. It is ill formed if not.
	void LoadSceneAssets(const nlohmann::json& assetData, const std::filesystem::path& root) {
		for (const uint64_t id : assetData) {
			assert(registry.exists(id));

			const AssetProperties properties = registry.getProperties(id);

			assert(id == properties.uuid);

			if (properties.type == AssetProperties::Type::Texture) {
				assert(!textures.contains(properties.uuid));

				assert(textures[properties.uuid].loadFromFile(std::format("{}/assets/textures/{}.png", root.string(), uint64_t(id)))); //Note: stick a std::string in the union as name may not map to file?
				//Potentially move files.
			}
			else if (properties.type == AssetProperties::Type::SubTexture) {
				const SubTextureMetadata subData = properties.extraneous.subTextureData;
				const auto parent = subData.parentUUID;

				if (!textures.contains(parent)) {
					assert(textures[parent].loadFromFile(std::format("{}/assets/textures/{}.png", root.string(), parent)));
				}

				TextureData data;
				data.parent = parent;
				data.texture.rect = subData.rect;
				data.texture.texture = &textures[parent];
				subTextures[uint64_t(id)] = data;
			}
			else {
				assert(false);
			}
		}
	}

	/*
		These two methods have zero to do with runtime loading of images. This is designed to register an asset as existing, and allow for runtime loading.
		For runtime loading, use AssetManager::GetTexture(uint64_t) or AssetManager::GetTexture(std::string).
	*/

	//Precondition: path exists and is a path to a PNG. 
	void ImportTexture(const std::filesystem::path& path, const std::string& name, const std::filesystem::path& projectRoot) {
		//Validate
		assert(path.extension() == ".png");
		assert(std::filesystem::exists(path));
		const bool valid = sf::Texture().loadFromFile(path.string());
		assert(valid);


		//Create
		const auto uuid = generateUUID();

		AssetProperties properties;
		properties.type = AssetProperties::Type::Texture;
		properties.uuid = uuid;

		{

			assert(std::filesystem::exists(projectRoot / std::filesystem::path("assets/"))); //Checks that we have an asset folder. Todo: Automate asset folder creation.
			const std::filesystem::path newPath = std::format("{}/assets/textures/{}.png", projectRoot.string(), uuid);

			std::filesystem::copy(path, newPath); //Can fail (throws).
		}

		registry.insert(name, properties, uuid);

		//Save to disk (maintain durability).

		std::ofstream out(projectRoot / "assetregistry.json");
		out << registry.SaveRegistry();
	}

	void ImportSubTextures(const uint64_t spriteSheetUUID, const SpritesheetData& spriteSheetData, const std::filesystem::path& projectRoot) {
		assert(registry.exists(spriteSheetUUID));
		const AssetProperties parentProperties = registry.getProperties(spriteSheetUUID);

		const std::string& spritesheetName = registry.getName(spriteSheetUUID);

		assert(parentProperties.type == AssetProperties::Type::Texture);

		//assert(textures.contains(parentID)); Actually, doesn't have to be loaded in-memory at the time of registration. 

		for (int id = 0; id < spriteSheetData.rows * spriteSheetData.cols; id++) {
			const std::string name = std::format("{}_{}", spritesheetName, id);

			const int tu = id % spriteSheetData.tilesetRatio;
			const int tv = id / spriteSheetData.tilesetRatio;

			const sf::IntRect subset(tu * spriteSheetData.tileSize, tv * spriteSheetData.tileSize, spriteSheetData.tileSize, spriteSheetData.tileSize);

			const SubTextureMetadata extraneous(spriteSheetUUID, subset);
			const uint64_t uuid = generateUUID();

			AssetProperties properties;
			properties.extraneous.subTextureData = extraneous;
			properties.uuid = uuid;
			properties.type = AssetProperties::Type::SubTexture;

			registry.insert(name, properties, uuid); //Doesn't load, just states existence.
		}

		//Save to disk (maintain durability).

		std::ofstream out(projectRoot / "assetregistry.json");
		out << registry.SaveRegistry();
	}

private:
	struct TextureData {
		Texture texture;
		uint64_t parent;
	};

	std::unordered_map<uint64_t, TextureData> subTextures;
	std::unordered_map<uint64_t, sf::Texture> textures;



	AssetRegistry registry;
};
