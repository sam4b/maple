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
	virtual void FromJson(const nlohmann::json& json, Systems context) noexcept = 0;
	[[nodiscard]] virtual nlohmann::json ToJson() const noexcept = 0;
	virtual void ImGuiDisplay() noexcept {
		ImGui::Text("Unimplemented. Override ImGuiDisplay().");
	}
	virtual ~ComponentMetadata() = default;
};