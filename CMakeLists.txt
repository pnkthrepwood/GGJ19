cmake_minimum_required (VERSION 3.0)
project (game)

set(CMAKE_CXX_STANDARD 11)

# game related

include_directories(gaem)
file(GLOB_RECURSE SOURCES gaem/*.cpp)

add_executable(game ${SOURCES})

target_link_libraries(game sfml-window sfml-system sfml-graphics sfml-audio GL)


file(GLOB DATA_FILES
  "data/*"
)
file(COPY ${DATA_FILES} DESTINATION ${CMAKE_BINARY_DIR})
