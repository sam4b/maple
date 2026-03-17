include (FetchContent)

set(CMAKE_CXX_STANDARD 20)

add_library(Maple STATIC "AssetManager.cpp" "AssetRegistry.cpp" "Entity.cpp" "EntityManager.cpp" "View.hpp" "ECSStorage.hpp" "Query.hpp" "maple.cpp" "Registry.cpp" "SceneParser.cpp" "Meta.cpp")
target_include_directories(Maple PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


FetchContent_Declare(
    json
    URL "https://github.com/nlohmann/json/archive/refs/tags/v3.12.0.zip"
)
FetchContent_MakeAvailable(json)


#credit to https://stackoverflow.com/a/65620000 for teaching me how to use fetchcontent with non-cmake projects 
FetchContent_Declare(
    imgui
    URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.92.6.zip"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
FetchContent_Populate(imgui)
add_library(imgui INTERFACE ${imgui_SOURCE_DIR})
target_include_directories(imgui INTERFACE ${imgui_SOURCE_DIR})
target_include_directories(imgui INTERFACE ${imgui_SOURCE_DIR}/misc/cpp) #include imgui_stdlib.hpp
set(IMGUI_DIR ${imgui_SOURCE_DIR}) #for imgui_sfml

FetchContent_Declare(
  SFML
  URL "https://github.com/SFML/SFML/archive/3.0.0.zip"
)

option(SFML_BUILD_AUDIO "Build audio" ON)
option(SFML_BUILD_NETWORK "Build network" OFF)
FetchContent_MakeAvailable(sfml)

FetchContent_Declare(
    imgui_sfml
    URL "https://github.com/sam4b/imgui-sfml/archive/refs/heads/master.zip" #my fork that makes it compatible with newer versions of imgui (api change in imgui)
)
FetchContent_MakeAvailable(imgui_sfml)


FetchContent_Declare(
    magic_enum
URL "https://github.com/Neargye/magic_enum/archive/refs/tags/v0.9.7.zip"
)
FetchContent_MakeAvailable(magic_enum)

target_link_libraries(Maple nlohmann_json::nlohmann_json imgui ImGui-SFML::ImGui-SFML sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    magic_enum::magic_enum
)
