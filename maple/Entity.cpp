#include "Entity.hpp"

Systems Entity::context = Systems();

Entity::Entity() {
	assert(false);
	//bad
}
Entity Entity::createEntity() {
	const uint64_t id = context.entityManager->create();

	return Entity(id);
}

[[nodiscard]] uint64_t Entity::getID() const noexcept {
	return id;
}