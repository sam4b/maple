#pragma once
#include <functional>
#include "Meta.hpp"
#include "ComponentMap.hpp"
#include <imgui.h>
#include "EntityManager.hpp"

class Script;
class Scene;

struct ComponentData {
    int typeID;
    std::function<ComponentMap* ()> storageFactory;
    std::function<void(EntityManager&, const uint64_t)> instanceFactory;
    std::function<void(const std::string&, EntityManager&, const uint64_t)> editorShowable;
};

struct UserContents {
    std::unordered_map<std::string, ComponentData> mapFactory;
    std::unordered_map<std::string, std::function<Script* ()>> scriptFactory;
    std::unordered_map<std::string, std::function<Scene* ()>> sceneFactory;
};

/*
 A singleton to be used to register user code (i.e. scripts, components) to the engine.
*/
class Registry {
public:
    template <typename T>
    requires std::is_base_of_v<ComponentMetadata, T>
    static void RegisterComponent(const std::string& string, std::function<ComponentMap* ()> mapFactory, std::function<void(T&)> ImGuiFunc) {
        if (!m_instance) {
            m_instance = new Registry();
        }

        assert(m_instance);

        if (m_instance->mapFactory.contains(string)) { //Some prior linking shit has done it before.
            return;
        }

        assert(!m_instance->mapFactory.contains(string));

        //Produce ImGui code


        const std::function<void(const std::string&, EntityManager&, const uint64_t)> ImGuiComponentCode = [](const std::string& s, EntityManager& manager, const uint64_t id) -> void {
            bool hasComponent = manager.hasComponent<T>(id);

            if (!hasComponent) return;

            if (ImGui::CollapsingHeader(s.c_str())) {
                bool shouldDelete = ImGui::Button("Delete");

                manager.getComponent<T>(id).ImGuiDisplay();

                if (shouldDelete) {
                    manager.removeComponent<T>(id);
                }
            }
            };


        const std::function<void(EntityManager&, const uint64_t)> instanceFactory = [](EntityManager& manager, const uint64_t id) -> void {
            if (manager.hasComponent<T>(id)) return;
            manager.addComponent<T>(id);
            };

        TypeIdentifier::registerType<T>();

        m_instance->mapFactory[string] = { TypeIdentifier::get<T>(), mapFactory, instanceFactory, ImGuiComponentCode };

    }

    template <typename T>
    static void RegisterScript(const std::string scriptName, std::function<Script* ()> scriptFactory) {
        if (!m_instance) {
            m_instance = new Registry();
        }

        assert(m_instance);

        assert(!m_instance->scriptFactory.contains(scriptName));

        m_instance->scriptFactory[scriptName] = scriptFactory;
    }

    static inline void RegisterScene(const std::string sceneName, std::function < Scene* ()> sceneSpawner) {
        if (!m_instance) {
            m_instance = new Registry();
        }

        assert(m_instance);

        assert(!m_instance->sceneFactory.contains(sceneName));

        m_instance->sceneFactory[sceneName] = sceneSpawner;

    }

    static inline UserContents GetUserContents() {
        if (!m_instance) {
            //Warn that user has registered nothing.
            m_instance = new Registry();
        }

        assert(m_instance);

        return {
            m_instance->mapFactory,
            m_instance->scriptFactory,
            m_instance->sceneFactory
        };
    }
private:
    static Registry* m_instance;

    std::unordered_map<std::string, ComponentData> mapFactory;
    std::unordered_map<std::string, std::function<Script* ()>> scriptFactory;
    std::unordered_map<std::string, std::function<Scene* ()>> sceneFactory;
};

#define REGISTER_COMPONENT(name) \
static_assert(std::is_base_of_v<ComponentMetadata, name>, "Component must implement (de)serialization functions!"); \
static struct maple_register_component_##name { maple_register_component_##name() { \
     Registry::RegisterComponent<name>(#name, []() -> ComponentMap* { return new ComponentMapImpl<name>();}, [](name& data) -> void { data.ImGuiDisplay();}); \
}\
\
} maple_internal_register_component_##name;


#define REGISTER_SCRIPT(name) \
static_assert(std::is_base_of_v<Script, name>, "Script must inherit from Script interface."); \
static struct maple_register_script_##name { maple_register_script_##name() { \
    Registry::RegisterScript<name>(#name, []() -> Script* { return new name; }); \
} \
} maple_internal_register_script_##name;

#define REGISTER_SCENE(name) \
static_assert(std::is_base_of_v<Scene, name>, "Scene must inherit from Scene interface"); \
static struct maple_register_scene_##name { maple_register_scene_##name() { \
   Registry::RegisterScene(#name, []() -> Scene* { return new name();});\
}} maple_internal_register_scene_##name;