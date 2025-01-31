#pragma once
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <cassert>
#include <queue>
#include <imgui.h>
#include <imgui-SFML.h>
#include <nlohmann/json.hpp>
#include <cassert>
#include <iostream>
#include <string>
#include <fstream>
#include "Scripting.hpp"
#include "EntityManager.hpp"
#include "Meta.hpp"
#include "SceneParser.hpp"
#include <magic_enum.hpp>

struct MapleProject {
    std::filesystem::path root;
    std::string name;
    std::filesystem::path entryPoint;
};

struct Maple {
    Systems systems;
    UserContents userConents;
    Scene* currentScene;
};

Maple LoadProject(const MapleProject& project);

std::queue<sf::Event> readEvents(sf::RenderWindow& window, bool& shouldClose);
/*
    Somehow this has gained responsibility for drawing...
*/
void step(Scene* scene, sf::Time time, RenderTarget& target, std::queue<sf::Event> events, Systems context, sf::Vector2i mousePos);


void runProject(const MapleProject& project);

void log(const std::string& string);

MapleProject OpenProject(const std::filesystem::path& path);

int maple_main(const std::filesystem::path& path);