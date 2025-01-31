#pragma once

class ScriptManager;
class AssetManager;
class EntityManager;
  
struct Systems {
	ScriptManager* scriptManager;
	EntityManager* entityManager;
	AssetManager* assetManager;
};