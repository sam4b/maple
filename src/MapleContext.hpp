#pragma once

class ScriptManager;
class AssetManager;
class EntityManager;

struct MapleServices {
	ScriptManager* scriptManager;
	EntityManager* entityManager;
	AssetManager* assetManager;
};