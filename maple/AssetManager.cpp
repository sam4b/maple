#include "AssetManager.hpp"

AssetManager::AssetManager(const std::filesystem::path& path) : projectRoot(path) {
}

void AssetManager::LoadRegistry(const nlohmann::json& json) noexcept {
	registry.LoadRegistry(json);
}

/*Need to find a nice way to supply this to only who I want (friend class?)*/
const AssetRegistry& AssetManager::GetRegistry() const noexcept {
	return registry;
}

nlohmann::json AssetManager::SaveRegistry() const noexcept {
	return registry.SaveRegistry();
}
/*
	Returns an texture if the texture has been imported into Maple.
	This method attempts to load the texture from disk if it has been imported but is not in memory.
*/
std::optional<Texture> AssetManager::GetTexture(const uint64_t id) {
	if (subTextures.contains(id)) {
		const TextureData& textureData = subTextures.at(id);

		assert(textures.contains(textureData.parent)); //parent must be loaded

		assert(textureData.texture.texture); //Non-null.

		return { textureData.texture };
	}
	else if (textures.contains(id)) {
		sf::Texture& texture = textures.at(id);

		return Texture {
			.texture = &texture,
			.rect = sf::IntRect{0, 0, (int)texture.getSize().x, (int)texture.getSize().y}
		};
	}

	//Not loaded

	const AssetProperties properties = registry.getProperties(id);

	if (properties.type == AssetProperties::Type::Texture) {
		LoadTexture(id);

		assert(textures.contains(id));

		sf::Texture& texture = textures.at(id);

		return Texture {
			.texture = &texture,
			.rect = sf::IntRect{0, 0, (int)texture.getSize().x, (int)texture.getSize().y}
		};
	}
	else if (properties.type == AssetProperties::Type::SubTexture) { //Tile in a spritesheet
		const uint64_t parentID = properties.extraneous.subTextureData.parentUUID;

		LoadSpritesheet(parentID);

		TextureData data;
		data.parent = parentID;
		data.texture.rect = properties.extraneous.subTextureData.rect;
		data.texture.texture = &textures.at(parentID);

		subTextures[id] = data;

		return subTextures.at(id).texture;
	}
	else {
		//Log and return an empty optional.
	}

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

std::optional<Texture> AssetManager::GetTexture(const std::string& path) {
	assert(registry.exists(path));
	return GetTexture(registry.getProperties(path).uuid);
}

const Animation& AssetManager::GetAnimation(const uint64_t uuid) {
	return animations.at(uuid);
}

const Animation& AssetManager::GetAnimation(const std::string& path) {
	const auto properties = registry.getProperties(path);
	return GetAnimation(properties.uuid);
}

//Must be called after loading the registry. It is ill formed if not.
void AssetManager::LoadSceneAssets(const nlohmann::json& assetData, const std::filesystem::path& root) {
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
			subTextures[id] = data;
		}
		else {
			assert(false);
		}
	}
}

void AssetManager::ImportAnimation(const std::string& name, const Animation data) {
	//Save to disk
	assert(data.ids.size() >= 1);
	assert(registry.exists(data.parent));
	assert(registry.getProperties(data.parent).type == AssetProperties::Type::Spritesheet);
	assert(data.frameTime >= 0);

	const auto uuid = generateUUID();
	AssetProperties properties;
	properties.type = AssetProperties::Type::Animation;
	properties.uuid = uuid;
	registry.insert(name, properties, uuid);

	animations.insert({ uuid, data });

	const std::filesystem::path path(std::format("{}/assets/animations/animation.json", projectRoot.string()));

	if (!std::filesystem::exists(path)) {
		std::ofstream out(path);
		nlohmann::json json;
		json["animations"][0] = data.toJson();
		json["animations"][0]["uuid"] = uuid;
		out << json;
	}
	else {
		std::ifstream in(path);
		nlohmann::json json = nlohmann::json::parse(in);
		in.close();
		json["animations"].push_back(data.toJson());
		json["animations"].back()["uuid"] = uuid;
		std::ofstream out(path);
		out << json;
	}
}

void AssetManager::ImportSoundEffect(const std::string& name, const std::filesystem::path& path) {
	//Validate
	assert(path.extension() == ".ogg");
	assert(std::filesystem::exists(path));
	const bool valid = sf::SoundBuffer().loadFromFile(path.string());
	assert(valid);

	assert(false);
}

void AssetManager::ImportMusic(const std::string& name, const std::filesystem::path& path) {
	//Validate
	assert(path.extension() == ".ogg");
	assert(std::filesystem::exists(path));
	const bool valid = sf::Music().openFromFile(path.string());
	assert(valid);

	assert(false);
}

void AssetManager::ImportFont(const std::string& name, const std::filesystem::path& path) {
	//Validate
	assert(path.extension() == ".ttf");
	assert(std::filesystem::exists(path));
	const bool valid = sf::Font().loadFromFile(path.string());
	assert(valid);

	assert(false);
}

void AssetManager::ImportSpritesheet(const std::filesystem::path& path, const std::string& name, const SpritesheetData data) {
	//Validate
	assert(path.extension() == ".png");
	assert(std::filesystem::exists(path));
	const bool valid = sf::Texture().loadFromFile(path.string());
	assert(valid);


	//Create
	const auto uuid = generateUUID();

	AssetProperties properties;
	properties.type = AssetProperties::Type::Spritesheet;
	properties.uuid = uuid;
	properties.extraneous.spriteSheetData = data;

	{

		if (!HasAssetFolders(projectRoot)) {
			CreateFolders(projectRoot);

		}

		const std::filesystem::path newPath = std::format("{}/assets/textures/{}.png", projectRoot.string(), uuid);

		std::filesystem::copy(path, newPath); //Can fail (throws).
	}



	registry.insert(name, properties, uuid);
	LoadSpritesheet(uuid);
	ImportSubTextures(uuid, data);
}


bool AssetManager::HasAssetFolders(const std::filesystem::path& path) const noexcept {
	return std::filesystem::exists(projectRoot / std::filesystem::path("assets/")) && std::filesystem::exists(projectRoot / std::filesystem::path("assets/textures/"))
		&& std::filesystem::exists(projectRoot / std::filesystem::path("assets/animations/"));
}

void AssetManager::CreateFolders(const std::filesystem::path& path) noexcept {
	std::filesystem::create_directories(path / "assets");
	std::filesystem::create_directories(path / "assets" / "textures/");
	std::filesystem::create_directories(path / "assets" / "animations/");
}


void AssetManager::LoadFont(const uint64_t uuid)
{

}



void AssetManager::LoadTexture(const uint64_t uuid) {
	assert(registry.exists(uuid));

	assert(registry.getProperties(uuid).type == AssetProperties::Type::Texture);

	if (textures.contains(uuid)) return;

	assert(textures[uuid].loadFromFile(std::format("{}/assets/textures/{}.png", projectRoot.string(), uuid)));
}

void AssetManager::LoadSubTexture(const uint64_t uuid) {
	assert(registry.exists(uuid));
	const auto properties = registry.getProperties(uuid);
	assert(properties.type == AssetProperties::Type::SubTexture);
	const auto data = properties.extraneous.subTextureData;
	LoadSpritesheet(data.parentUUID);
	assert(textures.contains(data.parentUUID));

	if (subTextures.contains(uuid)) return;


	TextureData tex;
	tex.parent = data.parentUUID;
	tex.texture.rect = data.rect;
	tex.texture.texture = GetTexture(data.parentUUID).value().texture;

	subTextures[uuid] = tex;
}

/*Does *not* load the tiles into memory, it only loads the texture they depend on.*/
void AssetManager::LoadSpritesheet(const uint64_t uuid) {
	assert(registry.getProperties(uuid).type == AssetProperties::Type::Spritesheet);

	if (textures.contains(uuid)) {
		return;
	}

	const auto success = textures[uuid].loadFromFile(std::format("{}/assets/textures/{}.png", projectRoot.string(), uuid));

	assert(success);
}

void AssetManager::LoadAllAnimations(const std::filesystem::path& path) {
	if (!std::filesystem::exists(path)) {
		std::cout << "[warn]: No animation json file. If the project has animations, they have not been located.\n";
		return;
	}

	std::ifstream f(path);
	assert(f.is_open());

	const nlohmann::json json = nlohmann::json::parse(f);

	for (const auto& animation : json["animations"]) {

		const uint64_t uuid = animation["uuid"];

		assert(!animations.contains(uuid)); //Duplicates.

		Animation a = Animation::fromJson(animation);

		animations[uuid] = a;
	}
}

void AssetManager::LoadAllAssetsInRegistry(const std::filesystem::path& projectRoot) {
	for (const auto& [name, properties] : registry.getAllAssets()) {
		const auto uuid = properties.uuid;
		//This is bad, I should store it in a per type array.
		switch (properties.type) {
			using enum AssetProperties::Type;
		case Texture:
			LoadTexture(uuid);
			break;
		case SubTexture:
			LoadSubTexture(uuid);
			break;
		case Animation: //We just load all animations now tbh
			break;
		case Spritesheet:
			LoadSpritesheet(uuid);
			break;
		default:
			assert(false);
			break;
		}
	}

	LoadAllAnimations(std::format("{}/assets/animations/animation.json", projectRoot.string()));
}

//Precondition: path exists and is a path to a PNG. 
void AssetManager::ImportTexture(const std::filesystem::path& path, const std::string& name) {
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

	LoadTexture(uuid);
	//Save to disk (maintain durability).

	std::ofstream out(projectRoot / "assetregistry.json");
	out << registry.SaveRegistry();
}

void AssetManager::ImportSubTextures(const uint64_t spriteSheetUUID, const SpritesheetData& spriteSheetData) {
	assert(registry.exists(spriteSheetUUID));
	const AssetProperties parentProperties = registry.getProperties(spriteSheetUUID);

	const std::string& spritesheetName = registry.getName(spriteSheetUUID);

	assert(parentProperties.type == AssetProperties::Type::Spritesheet);

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

		LoadSubTexture(uuid);
	}

	//Save to disk (maintain durability).

	std::ofstream out(projectRoot / "assetregistry.json");
	out << registry.SaveRegistry();
}