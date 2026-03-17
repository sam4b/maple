#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <cassert>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

struct SubTextureMetadata {
	SubTextureMetadata(const uint64_t parentID, const sf::IntRect subset) : parentUUID(parentID), rect(subset) {};
	uint64_t parentUUID;
	sf::IntRect rect;
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
		case SubTexture:
			extraneous.subTextureData.~SubTextureMetadata();
			break;
		case Spritesheet:
			extraneous.spriteSheetData.~SpritesheetData();
			break;
		case Animation:
			break;
		default:
			assert(false);
			break;
		}
	}

	nlohmann::json toJson() const noexcept {
		nlohmann::json out;

		out["id"] = uuid;

		//Just use magic_enum 
		out["type"] = [&]() -> std::string { switch (type) {
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

		switch (type) {
			using enum AssetProperties::Type;

		case Texture:
			break;
		case SubTexture:
			out["extraneous"]["parentUUID"] = extraneous.subTextureData.parentUUID;
			out["extraneous"]["rect"] = { extraneous.subTextureData.rect.position.x, extraneous.subTextureData.rect.position.y,
			extraneous.subTextureData.rect.size.x, extraneous.subTextureData.rect.size.y };
			break;
		case Animation:
			break;
		case Spritesheet:
			out["extraneous"]["tileSize"] = extraneous.spriteSheetData.tileSize;
			break;
		}

		return out;
	}

	static inline AssetProperties fromJson(const nlohmann::json& jsonObj) {
		const uint64_t id = jsonObj["id"];

		const AssetProperties::Type type = [](const std::string& s) -> AssetProperties::Type {
			if (s == "Texture") {
				return AssetProperties::Type::Texture;
			}
			else if (s == "SubTexture") {
				return AssetProperties::Type::SubTexture;
			}
			else if (s == "Animation") {
				return AssetProperties::Type::Animation;
			}
			else if (s == "Spritesheet") {
				return AssetProperties::Type::Spritesheet;
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
			properties.extraneous.subTextureData.rect = sf::IntRect{ {jsonObj["extraneous"]["rect"][0], jsonObj["extraneous"]["rect"][1]}, {jsonObj["extraneous"]["rect"][2], jsonObj["extraneous"]["rect"][3]} };
			break;
		case Spritesheet:
			properties.extraneous.spriteSheetData.tileSize = jsonObj["extraneous"]["tileSize"];
			break;
		case Animation:
			break;
		default:
			assert(false);
			break;
		}
		return properties;
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
		case Spritesheet:
			this->extraneous.spriteSheetData = old.extraneous.spriteSheetData;
			break;
		case Animation:
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
		case Spritesheet:
			this->extraneous.spriteSheetData = old.extraneous.spriteSheetData;
			break;		
		case Animation:
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
	} extraneous; //Type implies union data
};
