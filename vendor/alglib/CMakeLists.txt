

file(GLOB SRCS
     "src/*.h"
     "src/*.cpp"
)

add_library(alglib STATIC ${SRCS})

target_include_directories( alglib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
