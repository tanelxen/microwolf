cmake_minimum_required(VERSION 3.28)
project(microwolf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Configure GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

set(CMAKE_XCODE_GENERATE_SCHEME OFF)

# Thirdparty libraries
add_subdirectory( vendor/glad )
add_subdirectory( vendor/glfw )
add_subdirectory( vendor/imgui )

set(SRC_COMMON
        src/main.cpp
        src/Shader.cpp
        src/Shader.h
        src/Camera.cpp
        src/Camera.h
        src/KeyValueCollection.cpp
        src/KeyValueCollection.h
        src/TextureAtlas.cpp
        src/TextureAtlas.h
        src/PlayerMovement.cpp
        src/PlayerMovement.h
        src/Player.cpp
        src/Player.h
        src/Monster.cpp
        src/Monster.h
        src/Footsteps.cpp
        src/Footsteps.h
        
        src/Application.cpp
        src/Application.h
        src/Input.cpp
        src/Input.h
        
        src/HUD.cpp
        src/HUD.h
        src/DebugRenderer.cpp
        src/DebugRenderer.h
        src/Cube.cpp
        src/Cube.h
        
        src/RenderDevice.cpp
        src/RenderDevice.h
)

set(SRC_Q3
        src/Q3BSPAsset.cpp
        src/Q3BSPAsset.h
        src/Q3BSPTypes.h
        src/Q3BspMesh.cpp
        src/Q3BspMesh.h
        src/Q3MapScene.cpp
        src/Q3MapScene.h
        src/Q3BspCollision.cpp
        src/Q3BspCollision.h
        src/Q3Bezier.cpp
        src/Q3Bezier.h
        src/Q3LightGrid.cpp
        src/Q3LightGrid.h
        src/Q3Shaders.cpp
        src/Q3Shaders.h
)

set(SRC_GLDSRC
        src/GoldSrcMDLAsset.cpp
        src/GoldSrcMDLAsset.h
        src/GoldSrcModel.cpp
        src/GoldSrcModel.h
)

set(SRC_MINIAUDIO
        src/miniaudio.c
        src/ma_reverb_node.cpp
        src/verblib.cpp
        src/verblib.h
)

set(SRC_IMGUI
        vendor/imgui/backends/imgui_impl_glfw.cpp
        vendor/imgui/backends/imgui_impl_opengl3.cpp
        #src/imguiStyling.cpp
)

add_executable( ${PROJECT_NAME}
        ${SRC_COMMON}
        ${SRC_Q3}
        ${SRC_GLDSRC}
        ${SRC_SOURCE}
        ${SRC_MINIAUDIO}
        ${SRC_IMGUI}
)

source_group("Common" FILES ${SRC_COMMON})
source_group("Quake 3" FILES ${SRC_Q3})
source_group("GoldSrc" FILES ${SRC_GLDSRC})
source_group("miniaudio" FILES ${SRC_MINIAUDIO})
source_group("imgui" FILES ${SRC_IMGUI})

target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_XYZW_ONLY)

target_include_directories(${PROJECT_NAME} PRIVATE vendor/glm)

target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw imgui)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/assets
)
