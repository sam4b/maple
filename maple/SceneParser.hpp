#pragma once
#include <nlohmann/json.hpp>
#include "ScriptManager.hpp"
#include "AssetManager.hpp"
#include "EntityManager.hpp"
#include "Scene.hpp"

Scene* ParseScene(const nlohmann::json& json, const std::unordered_map < std::string, std::function<Scene* ()>>& sceneFactory, Systems context, const std::filesystem::path& root);