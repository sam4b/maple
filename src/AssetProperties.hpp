#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <cassert>
#include <string>
#include <unordered_map>

struct SubTextureMetadata {
	SubTextureMetadata(const uint64_t parentID, const sf::IntRect subset) : parentUUID(parentID), rect(subset) {};
	uint64_t parentUUID;
	sf::IntRect rect;
};

struct AnimationData {
	AnimationData() : ids({}), frameTime(0) {};
	std::vector<uint64_t> ids;
	uint64_t frameTime; //millis
};

struct SpritesheetData {
	SpritesheetData() {
		rows = 0;
		cols = 0;
		tilesetRatio = 0;
		tileSize = 0;
	}
	int rows;
	int cols;
	int tilesetRatio;
	int tileSize;
};


struct AssetProperties {
	AssetProperties() = default;
	~AssetProperties() {
		switch (type) {
			using enum Type;
		case Texture:
			break;
		case Animation:
			extraneous.animatonData.~AnimationData();
			break;
		case SubTexture:
			extraneous.subTextureData.~SubTextureMetadata();
			break;
		case Spritesheet:
			extraneous.spriteSheetData.~SpritesheetData();
			break;
		default:
			assert(false);
			break;
		}
	}
	AssetProperties(const AssetProperties& old) {
		this->uuid = old.uuid;
		this->type = old.type;

		switch (type) {
			using enum Type;
		case Texture:
			break;
		case SubTexture:
			this->extraneous.subTextureData = old.extraneous.subTextureData;
			break;
		case Animation:
			this->extraneous.animatonData = old.extraneous.animatonData;
			break;
		case Spritesheet:
			this->extraneous.spriteSheetData = old.extraneous.spriteSheetData;
			break;
		default:
			assert(false);
			break;
		}
	}

	AssetProperties& operator=(const AssetProperties& old) {
		this->uuid = old.uuid;
		this->type = old.type;

		switch (type) {
			using enum Type;
		case Texture:
			break;
		case SubTexture:
			this->extraneous.subTextureData = old.extraneous.subTextureData;
			break;
		case Animation:
			this->extraneous.animatonData = old.extraneous.animatonData;
			break;
		case Spritesheet:
			this->extraneous.spriteSheetData = old.extraneous.spriteSheetData;
			break;
		default:
			assert(false);
			break;
		}

		return *this;
	}

	enum class Type {
		Texture,
		SubTexture,
		Animation,
		Spritesheet
	} type;

	uint64_t uuid;

	//move to std variant?
	union AssetData {
		AssetData() {
			memset(this, 0, sizeof(AssetData));
		}
		~AssetData() {

		}
		SubTextureMetadata subTextureData;
		SpritesheetData spriteSheetData;
		AnimationData animatonData;
	} extraneous; //Type implies union data
};

[[nodiscard]] AssetProperties fromJson(const nlohmann::json& jsonObj) noexcept {
	const uint64_t id = jsonObj["id"];

	const AssetProperties::Type type = [](const std::string& s) -> AssetProperties::Type {
 		if (s == "Texture") {
			return AssetProperties::Type::Texture;
		}
		else if (s == "SubTexture") {
			return AssetProperties::Type::SubTexture;
		}
		else {
			assert(false);
		}

		}(jsonObj["type"]);
	
	AssetProperties properties;
	properties.type = type;
	properties.uuid = id;

	switch (type) {
		using enum AssetProperties::Type;
	case Texture:
		break;
	case SubTexture:
		properties.extraneous.subTextureData.parentUUID = jsonObj["extraneous"]["parentUUID"];
		properties.extraneous.subTextureData.rect = sf::IntRect{ jsonObj["extraneous"]["rect"][0], jsonObj["extraneous"]["rect"][1], jsonObj["extraneous"]["rect"][2], jsonObj["extraneous"]["rect"][3] };
		break;
	case Spritesheet:
		properties.extraneous.spriteSheetData.tileSize = jsonObj["extraneous"]["tileSize"];
		break;
	case Animation:
		assert(false);
		break;
	default:
		assert(false);
		break;
	}
	return properties;
}

[[nodiscard]] nlohmann::json toJson(const AssetProperties& properties) noexcept {
	nlohmann::json out;

	out["id"] = properties.uuid;

	//Just use magic_enum 
	out["type"] = [&]() -> std::string { switch (properties.type) {
		using enum AssetProperties::Type;

	case Texture:
		return "Texture";
	case SubTexture:
		return "SubTexture";
	case Animation:
		return "Animation";
	case Spritesheet:
		return "Spritesheet";
	default:
		assert(false);
	} } ();

	switch (properties.type) {
		using enum AssetProperties::Type;

	case Texture:
		break;
	case SubTexture:
		out["extraneous"]["parentUUID"] = properties.extraneous.subTextureData.parentUUID;
		out["extraneous"]["rect"] = { properties.extraneous.subTextureData.rect.getPosition().x, properties.extraneous.subTextureData.rect.getPosition().y,
		properties.extraneous.subTextureData.rect.getSize().x, properties.extraneous.subTextureData.rect.getSize().y };
		break;
	case Animation:
		assert(false);
		break;
	case Spritesheet:
		out["extraneous"]["tileSize"] = properties.extraneous.spriteSheetData.tileSize;
		break;
	}

	return out;
}
