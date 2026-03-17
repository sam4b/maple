#pragma once
#include <SFML/Graphics.hpp>
#include "Registry.hpp"
#include <imgui.h>
#include <imgui-SFML.h>
#include <imgui_stdlib.h>
#include "AssetManager.hpp"


struct SpriteComponent : ComponentMetadata {
	sf::RectangleShape rectangle;
	uint64_t texture; //If rectangle.getTexture() is NULL, this is an invalid UUID. 

	void ImGuiDisplay() noexcept override {
		sf::RectangleShape& shape = rectangle;

		sf::Vector2f pos = shape.getPosition();
		sf::Vector2f size = shape.getSize();

		//Later texture?

		ImGui::InputFloat2("Position", (float*)&pos);
		ImGui::InputFloat2("Size", (float*)&size);

		ImGui::Image(*shape.getTexture(), sf::Vector2f{ 64, 64 });
		ImGui::SameLine();

		if (ImGui::Button("Update texture")) {

		}

		shape.setPosition(pos);
		shape.setSize(size);
	}


	nlohmann::json ToJson() const noexcept override {
		nlohmann::json json;
		if (rectangle.getTexture() != nullptr) {
			json["texture"] = texture;
		}

		json["pos"] = { rectangle.getPosition().x, rectangle.getPosition().y };
		json["size"] = { rectangle.getSize().x, rectangle.getSize().y };

		return json;
	}

	void FromJson(const nlohmann::json& json, Systems context) noexcept override {
		if (json.contains("texture")) {
			const uint64_t id = json["texture"];
			this->texture = id;
			const Texture texture = context.assetManager->GetTexture(id).value();
			this->rectangle.setTextureRect(texture.rect);
			this->rectangle.setTexture(texture.texture);
		}

		rectangle.setPosition({ json["pos"][0], json["pos"][1] });
		rectangle.setSize({ json["size"][0], json["size"][1] });
	}
};
REGISTER_COMPONENT(SpriteComponent)

struct Physics2DComponent : public ComponentMetadata {
	float mass;

	// Inherited via ComponentMetadata
	void FromJson(const nlohmann::json& json, Systems context) noexcept override
	{
	}
	nlohmann::json ToJson() const noexcept override
	{
		return nlohmann::json();
	}
};
REGISTER_COMPONENT(Physics2DComponent);

struct TransformComponent : ComponentMetadata {
	sf::Vector2f pos;
	sf::Vector2f velocity;

	void ImGuiDisplay() noexcept override {
		ImGui::InputFloat2("Position", (float*)&pos);
		ImGui::InputFloat2("Velocity", (float*)&velocity);
	}

	nlohmann::json ToJson() const noexcept override {
		return { /*{ "pos" , {component.pos.x, component.pos.y}}, {"velocity", {component.velocity.x, component.velocity.y}}*/};
	}

	void FromJson(const nlohmann::json& json, Systems context) noexcept override {
		sf::Vector2f pos = { json["pos"][0], json["pos"][1] };
		sf::Vector2f velocity = { json["velocity"][0], json["velocity"][1] };

		this->pos = pos;
		this->velocity = velocity;
	}
};
REGISTER_COMPONENT(TransformComponent);


struct AABBCollisionComponent : ComponentMetadata {
	sf::Vector2f pos;
	sf::Vector2f size;

	void ImGuiDisplay() noexcept override {
		ImGui::InputFloat2("Position", (float*)&pos);
		ImGui::InputFloat2("Size", (float*)&size);
	}

	nlohmann::json ToJson() const noexcept override {
		return { /*{ "pos" , {component.pos.x, component.pos.y}}, {"size", {component.size.x, component.size.y}}*/};
	}

	void FromJson(const nlohmann::json& json, Systems context) noexcept override {
		sf::Vector2f pos = { json["pos"][0], json["pos"][1] };
		sf::Vector2f size = { json["size"][0], json["size"][1] };

		this->pos = pos;
		this->size = size;
	}
};
REGISTER_COMPONENT(AABBCollisionComponent);


struct NameComponent : ComponentMetadata {
	std::string name;

	void ImGuiDisplay() noexcept {
		ImGui::InputText("Name", &name);
	}

	nlohmann::json ToJson() const noexcept override {
		return {"name", name };
	}

	void FromJson(const nlohmann::json& json, Systems context) noexcept override {
		name = json["name"];
	}


};
REGISTER_COMPONENT(NameComponent);