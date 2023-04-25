
add_library(imgui_file_dialog
  ImGuiFileDialog.cpp
  ImGuiFileDialog.h
  ImGuiFileDialogConfig.h
)

target_include_directories(imgui_file_dialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(imgui_file_dialog PUBLIC imgui)