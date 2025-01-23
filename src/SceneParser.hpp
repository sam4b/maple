#pragma once
#include <nlohmann/json.hpp>
#include "ScriptManager.hpp"
#include "AssetManager.hpp"
#include "EntityManager.hpp"
#include "Scene.hpp"


Scene* ParseScene(const nlohmann::json& json, const std::unordered_map < std::string, std::function<Scene*()>>& sceneFactory, MapleServices context) {
	assert(json.contains("sceneName"));
	
	const std::string sceneName = json["sceneName"];
	
	assert(sceneFactory.contains(sceneName));

	assert(json.contains("extraData"));
	const nlohmann::json& extraData = json["extraData"];

	Scene* scene = sceneFactory.at(sceneName)();

	assert(scene);

	assert(json.contains("assets"));
	context.assetManager->LoadSceneAssets(json["assets"]);

	assert(json.contains("entities"));
	context.entityManager->loadJSON(json, context);


	assert(json.contains("scripts"));

	for (const auto scripts : json["scripts"]) {
		const uint64_t id = scripts["entityID"];
		const Entity entity = *(Entity*)&id;

		const std::string scriptName = scripts["name"];
		context.scriptManager->addScript(entity, context, scriptName);
	};

	scene->onSceneCreate(context, extraData);


	return scene;
}
