cmake_minimum_required(VERSION 3.16)

project(clutter)

find_package(SDL3 REQUIRED)

# Embed assets into the binary using xxd
set(EMBED_C "${CMAKE_CURRENT_BINARY_DIR}/assets_embedded.c")
add_custom_command(
    OUTPUT "${EMBED_C}"
    COMMAND xxd -i level.txt "${EMBED_C}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/level.txt"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    COMMENT "Embedding assets/level.txt"
)

add_executable(clutter
    src/main.c
    src/game.c
    src/player.c
    src/enemy.c
    src/bullet.c
    src/tilemap.c
    src/renderer.c
    src/texture.c
    src/particle.c
    src/camera.c
    "${EMBED_C}"
)

target_include_directories(clutter PRIVATE src)
target_link_libraries(clutter SDL3::SDL3)
