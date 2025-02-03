#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <chrono>
#include <nlohmann/json.hpp>
#include <ranges>
#include "Registry.hpp"

struct Animation {
	uint64_t parent;
	std::vector<uint64_t> ids;  //index into tileset. we assume the rectangle is n by n where n is the tileSize of the tileset.
	//this impl allows for non adjacent tiles. this may change
	int frameTime; //milliseconds

	nlohmann::json toJson() const noexcept {
		nlohmann::json json;
		json["ids"] = ids;
		json["frameTime"] = frameTime;
		json["parent"] = parent;
		return json;
	}

	[[nodiscard]] static Animation fromJson(const nlohmann::json& json) noexcept {
		Animation data;
		data.frameTime = json["frameTime"];
		for (uint64_t id : json["ids"]) {
			data.ids.push_back(id);
		}
		data.parent = json["parent"];
		return data;
	}
};

//This implies a hash map lookup for each animation state component update. What if on transition we set the lastUpdate t threshold and subtracted
//dt each frame, and when lastUpdate < 0 then we do the lookup.
struct AnimationStateComponent : public ComponentMetadata {
	int lastUpdate;
	int offset;
	uint64_t animationID;
	uint64_t priorTexture;
	bool playForever;

	void FromJson(const nlohmann::json& json, Systems context) noexcept override {
		lastUpdate = json["lastUpdate"];
		offset = json["offset"];
		animationID = json["animationID"];
		playForever = json["playForever"];
		priorTexture = json["priorTexture"];
	}

	[[nodiscard]] nlohmann::json ToJson() const noexcept override {
		nlohmann::json json;
		json["offset"] = offset;
		json["animationID"] = animationID;
		json["lastUpdate"] = lastUpdate;
		json["playForever"] = playForever;
		json["priorTexture"] = priorTexture;
		return json;
	}

	void ImGuiDisplay() noexcept override {
		ImGui::Text("Unimplemented. Override ImGuiDisplay().");
	}
};
REGISTER_COMPONENT(AnimationStateComponent);

//If you call StopAnimation(entity), and a TransitionComponent is attached, the entity will either become that sprite, or begin a new animation.
struct TransitionComponent {
	uint64_t uuid;
	AssetProperties::Type type;
};