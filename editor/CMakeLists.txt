add_library(editor STATIC "editor.cpp")
target_include_directories(editor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(imgui CONFIG REQUIRED)
target_link_libraries(imguifilebrowser PRIVATE imgui::imgui)
target_link_libraries(editor PRIVATE imguifilebrowser)
target_link_libraries(editor PRIVATE Maple)