#pragma once
#include "Scripting.hpp"
#include "EntityManager.hpp"
#include "Meta.hpp"
#include "AssetManager.hpp"

enum class GameEvent {
	QueryInteraction
};

class LinkScript : public Script {
public:
	void onAttach(Systems& context) override {
		auto& trans = entity.addComponent<TransformComponent>();
		trans.velocity = { 0, 0 };
		trans.pos = { 300, 300 };
		auto& sprite = entity.addComponent<SpriteComponent>();
		sprite.rectangle.setSize({ 32,32 });
		sprite.rectangle.setPosition({ 0,0 });

		const auto variant = context.assetManager->GetTexture("blobOfMalice.png");
		assert(variant.has_value());

		const Texture texture = variant.value();
		sprite.rectangle.setTexture(texture.texture);
		sprite.rectangle.setTextureRect(texture.rect);

		auto& aabb = entity.addComponent<AABBCollisionComponent>();
		aabb.pos = trans.pos;
		aabb.size = sprite.rectangle.getSize();
	}

	void onDetach() override {

	}

	void onUpdate(const float dt, Systems& view, Scene* scene) override {
		sf::Vector2f vel{ 0,0 };
		sf::Event events;
		while (this->hasEvent(events)) {
			
		}

		constexpr float speed = 100;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			vel.x += speed;
			direction = Direction::Right;
		};
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			vel.x -= speed;
			direction = Direction::Left;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			vel.y -= speed;
			direction = Direction::Up;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			vel.y += speed;
			direction = Direction::Down;
		}

		if (scene->moveWouldCauseCollision(entity, vel, dt, view)) {
			vel = { 0, 0 };
		}
		
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
			//Check if entities exist around me
			//Query them for interaction if so

			const std::vector<Entity> near = scene->getEntitiesNear<SpriteComponent, TransformComponent>(entity, view, 500);


			for (Entity near : scene->getEntitiesNear<SpriteComponent, TransformComponent>(entity, view, 100000)) {
				if (view.scriptManager->hasScript(near) && near.getID() != entity.getID()) {
					Script* s = view.scriptManager->getScript(near);
					
					s->tryInteract(entity);
				}
			}
		}

		auto& trans = entity.getComponent<TransformComponent>();

		trans.velocity = vel;
	}
private:
	enum class Direction {
		Up, 
		Down,
		Left,
		Right
	} direction = Direction::Down;
};
REGISTER_SCRIPT(LinkScript);

class NPCScript : public Script {
public:
	void onAttach(Systems& context) override {
		auto& trans = entity.addComponent<TransformComponent>();
		trans.velocity = { 0, 0 };
		trans.pos = { 0,0};
		auto& sprite = entity.addComponent<SpriteComponent>();
		sprite.rectangle.setSize({ 32,32 });
		sprite.rectangle.setPosition(trans.pos);


		const auto variant = context.assetManager->GetTexture("blobOfMalice.png");
		assert(variant.has_value());

		const Texture texture = variant.value();
		sprite.rectangle.setTexture(texture.texture);
		sprite.rectangle.setTextureRect(texture.rect);


		auto& aabb = entity.addComponent<AABBCollisionComponent>();
		aabb.pos = trans.pos;
		aabb.size = sprite.rectangle.getSize();
	}

	void tryInteract(Entity interactor) override {
		display = true;
		std::cout << "Interacted!\n";
	}

	void onDetach() override {

	}

	void onUpdate(const float dt, Systems& view, Scene* scene) override {
		if (display) {
			scene->putDialog("There's a Skibidi in my bed\nSigma's gooning in my head\nGlizzies all over the room", 4000);
			display = false;
		}
	}
private:
	bool display = false;
};
REGISTER_SCRIPT(NPCScript);