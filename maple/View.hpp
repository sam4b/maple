#pragma once
#include "ECSStorage.hpp"

/*
	Lightweight, immutable and iterable view of one component.
	Used when you do not need the overhead of copying a component to join it, etc and merely need the value.
*/
template <typename T>
class ComponentView {
public:
	ComponentView(ECSStorage<T>* storage) : storage(storage) {};

	const std::vector<T>& iterate() const noexcept {
		return storage->data;
	}
private:
	ECSStorage<T>* storage;
};