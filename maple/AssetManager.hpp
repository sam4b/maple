#pragma once
#include <SFML/Graphics.hpp>
#include <variant>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <format>
#include <imgui.h>
#include <magic_enum.hpp>
#include "imgui_stdlib.h"
#include "Utils.hpp"
#include "AssetRegistry.hpp"
#include <SFML/Audio.hpp>
#include "Animation.hpp"

struct Texture {
	sf::Texture* texture;
	sf::IntRect rect;
};


/*
	Consider ref-counted handle system. Also support for other types of assets + subtextures (tilemaps?).
*/
class AssetManager {
public:
	AssetManager() = delete;
	AssetManager(const std::filesystem::path& path);

	void LoadRegistry(const nlohmann::json& json) noexcept;

	/*Need to find a nice way to supply this to only who I want (friend class?)*/
	const AssetRegistry& GetRegistry() const noexcept;

	nlohmann::json SaveRegistry() const noexcept;
	/*
		Returns an texture if the texture has been imported into Maple.
		This method attempts to load the texture from disk if it has been imported but is not in memory.
	*/
	std::optional<Texture> GetTexture(const uint64_t id);

	std::optional<Texture> GetTexture(const std::string& path);

	const Animation& GetAnimation(const std::string& path);

	const Animation& GetAnimation(const uint64_t path);

	//Must be called after loading the registry. It is ill formed if not.
	void LoadSceneAssets(const nlohmann::json& assetData, const std::filesystem::path& root);

	/*
		These two methods have zero to do with runtime loading of images. This is designed to register an asset as existing, and allow for runtime loading.
		For runtime loading, use AssetManager::GetTexture(uint64_t) or AssetManager::GetTexture(std::string).
	*/

	void ImportAnimation(const std::string& name, const Animation data);

	void ImportSoundEffect(const std::string& name, const std::filesystem::path& path);

	void ImportMusic(const std::string& name, const std::filesystem::path& path);

	void ImportFont(const std::string& name, const std::filesystem::path& path);

	void ImportSpritesheet(const std::filesystem::path& path, const std::string& name, const SpritesheetData data);


	void LoadAllAssetsInRegistry(const std::filesystem::path& projectRoot);

	//Precondition: path exists and is a path to a PNG. 
	void ImportTexture(const std::filesystem::path& path, const std::string& name);
private:
	struct TextureData {
		Texture texture;
		uint64_t parent;
	};

	//Seperated due to diff meaning.

	void ImportSubTextures(const uint64_t spriteSheetUUID, const SpritesheetData& spriteSheetData);

	void LoadFont(const uint64_t uuid);

	void LoadTexture(const uint64_t uuid);

	void LoadSubTexture(const uint64_t uuid);

	void LoadSpritesheet(const uint64_t uuid);

	void LoadAllAnimations(const std::filesystem::path& path);

	bool HasAssetFolders(const std::filesystem::path& path) const noexcept;

	void CreateFolders(const std::filesystem::path& path) noexcept;

	//TODO: Potentially just calculate off the fly tbh by (uuid, id). Profile memory and cpu usage to decide.
	std::unordered_map<uint64_t, TextureData> subTextures;

	std::unordered_map<uint64_t, sf::Texture> textures;

	std::unordered_map<uint64_t, Animation> animations;

	std::filesystem::path projectRoot;

	AssetRegistry registry;

	friend bool AnimationImport(AssetManager&);
	friend void Assets(AssetManager&, float);
};
