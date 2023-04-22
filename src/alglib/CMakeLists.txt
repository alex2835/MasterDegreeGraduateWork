

file(GLOB SRCS
     "src/*.h"
     "src/*.cpp"
)

add_library(alglib STATIC ${SRCS})

target_include_directories( alglib PUBLIC ${SRCS} )