include(FetchContent)

FetchContent_Declare(
  doctest
  GIT_REPOSITORY "https://github.com/onqtam/doctest.git"
  GIT_TAG v2.4.9
)
add_subdirectory(doctest)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
  GIT_TAG 9.1.0
)
add_subdirectory(fmt)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
  GIT_TAG c191faf0ba478e9c58a69c63306986a21ebfb6e4 # docking-latest
)
add_subdirectory(imgui)

FetchContent_Declare(
  implot
  GIT_REPOSITORY "https://github.com/ocornut/implot.git"
)
add_subdirectory(implot)

FetchContent_Declare(
  SDL2
  GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
  GIT_TAG release-2.26.1
)
add_subdirectory(sdl)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
  GIT_TAG v1.11.0
)
add_subdirectory(spdlog)
