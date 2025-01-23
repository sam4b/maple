
set(CMAKE_CXX_STANDARD 20)


add_executable(Maple "Maple.cpp" "SceneParser.hpp" "Map.hpp" "vendor/ImGuiFileBrowser.cpp" "Random.hpp" "Registry.hpp" "AssetManager.hpp" "Scene.hpp" "Tileset.hpp" "Menu.hpp" "Meta.hpp" "ComponentMap.hpp" "GameScene.hpp" "ScriptManager.hpp" "EditorScene.hpp" "LinkScript.hpp" "Animation.hpp" "Entity.hpp" "EntityManager.hpp" "RenderTarget.hpp" "Windows.hpp" "MapleComponents.hpp" "Scripting.hpp" "vendor/ImGuiFileBrowser.cpp")


include(FetchContent)

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glaze)

target_link_libraries(Maple PRIVATE glaze::glaze)

find_package(SFML COMPONENTS system window graphics audio CONFIG REQUIRED)
target_link_libraries(Maple PUBLIC sfml-system sfml-network sfml-audio sfml-graphics sfml-window)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Maple PUBLIC nlohmann_json::nlohmann_json)

find_package(ImGui-SFML CONFIG REQUIRED)
target_link_libraries(Maple PUBLIC ImGui-SFML::ImGui-SFML)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(Maple PUBLIC imgui::imgui)

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(Maple PRIVATE magic_enum::magic_enum)
