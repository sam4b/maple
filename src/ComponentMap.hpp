#pragma once
#include <cstdint>
#include <imgui.h>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "MapleContext.hpp"

/*
 * Not used for actual dynamic dispatch, just used to enforce some behaviours for components. (Perhaps concepts?).
*/
class ComponentMetadata {
public:
	virtual void FromJson(const nlohmann::json& json, MapleServices context) noexcept = 0;
	[[nodiscard]] virtual nlohmann::json ToJson() const noexcept = 0;
	virtual void ImGuiDisplay() noexcept {
		ImGui::Text("Unimplemented. Override ImGuiDisplay().");
	}
	virtual ~ComponentMetadata() = default;
};


class ComponentMap {
public:
	virtual ~ComponentMap() = default;
	[[nodiscard]] virtual void* getMap() noexcept = 0;

	virtual void destroyIfContains(uint64_t id) noexcept = 0;

	virtual void fromJSON(const nlohmann::json& json, MapleServices context) noexcept = 0;

	virtual nlohmann::json toJSON() const noexcept = 0;
};

template <typename Component>
requires std::is_base_of_v<ComponentMetadata, Component>
class ComponentMapImpl final : public ComponentMap {
public:
	[[nodiscard]] void* getMap() noexcept override {
		return &map;
	}

	void destroyIfContains(uint64_t id) noexcept override {
		if (map.contains(id)) {
			map.erase(id);
		}
	}

	void fromJSON(const nlohmann::json& json, MapleServices context) noexcept override {
		for (const auto& jsonObj : json) {
			const uint64_t id = jsonObj["id"];

			Component c;
			c.FromJson(jsonObj["data"], context);

			map.insert({ id,  c});
		}
	}

	nlohmann::json toJSON() const noexcept override {
		nlohmann::json out;
		for (const auto& [id, data] : map) {
			nlohmann::json j;
			j["id"] = id;
			j["data"] = data.ToJson();
			out.push_back(j);
		}
		return out;
	}

private:
	std::unordered_map<uint64_t, Component> map;

};