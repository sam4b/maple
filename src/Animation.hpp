#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <chrono>
#include <ranges>

#include "Tileset.hpp"
#include "EntityManager.hpp"

struct Animation {
	uint64_t tilesetID;
	std::vector<int> ids;  //index into tileset. we assume the rectangle is n by n where n is the tileSize of the tileset.
	//this impl allows for non adjacent tiles. this may change
	int frameTime; //milliseconds
	//rt
};

struct AnimationStateComponent {
	int lastUpdate;
	int offset;
	uint64_t animationID;
};

void animation_update(EntityManager& manager) {
	for (Entity entity : manager.getEntities() | std::ranges::views::transform([&](uint64_t id) -> Entity {
		return manager.getEntity(id);
		}) | std::ranges::views::filter([&](Entity entity) -> bool {
			return true /*manager.hasComponent<AnimationStateComponent> && manager.hasComponent<SpriteComponent>;*/;
			})) {


	}
}