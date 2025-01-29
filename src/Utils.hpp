#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <imgui.h>


inline sf::Vector2i getRelativeToLatestImGuiTopLeft(const sf::Vector2i mousePos) {
	const ImVec2 beginCorner = ImGui::GetCursorScreenPos();
	const sf::Vector2i corner = { (int)beginCorner.x, (int)beginCorner.y };

	const sf::Vector2i index = mousePos - corner;

	return index;
}

inline void createGrid(const unsigned int tileSize, const sf::Vector2u texSize, std::vector<sf::Vertex>& outBuffer) {
	outBuffer.clear();
	for (int row = 0; row < texSize.y / tileSize; row++) {
		const auto begin = sf::Vertex(sf::Vector2f(0, row * tileSize));
		const auto end = sf::Vertex(sf::Vector2f(texSize.x, row * tileSize));

		outBuffer.push_back(begin);
		outBuffer.push_back(end);
	}
	for (int col = 0; col < texSize.x / tileSize; col++) {
		const auto begin = sf::Vertex(sf::Vector2f(col * tileSize, 0));
		const auto end = sf::Vertex(sf::Vector2f(col * tileSize, texSize.y));

		outBuffer.push_back(begin);
		outBuffer.push_back(end);
	}
}

inline int mapVectorToID(const sf::Vector2i vec, const int tileSize) {
	return (vec.x % (tileSize + 1)) + (vec.y * (tileSize + 1));
}