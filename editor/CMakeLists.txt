
set(CMAKE_CXX_STANDARD 20)

add_library(editor STATIC "editor.cpp" "AssetGUI.hpp")
target_include_directories(editor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(maple)
target_link_libraries(editor PRIVATE Maple)

find_package(imgui-filebrowser)
target_link_libraries(editor PRIVATE imgui-filebrowser)