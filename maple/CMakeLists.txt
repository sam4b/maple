
set(CMAKE_CXX_STANDARD 20)

add_library(Maple STATIC "AssetManager.cpp" "AssetRegistry.cpp" "Entity.cpp" "EntityManager.cpp" "View.hpp" "ECSStorage.hpp" "Query.hpp" "maple.cpp" "Registry.cpp" "SceneParser.cpp" "Meta.cpp")
target_include_directories(Maple PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

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
