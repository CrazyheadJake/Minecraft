
cmake_minimum_required(VERSION 3.26.3)

# Unset these to statically link the C++ STL, important for if we are shipping the program 
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

# add_subdirectory("lib/")

project(Minecraft)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Enable AddressSanitizer if supported
option(ENABLE_ASAN "Enable AddressSanitizer" ON)

if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()

# we define MY_SOURCES to be a list of all the source files for my project
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${CMAKE_PROJECT_NAME} ${MY_SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")


# target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/") # This is useful to get an ASSETS_PATH in your IDE during development but you should comment this if you compile a release version and uncomment the next line
#target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/") # Uncomment this line to setup the ASSETS_PATH macro to the final assets directory when you share the project

# === Platform-specific configuration ===

if(WIN32)
    # Windows settings
    set(RAYLIB_LIB "${CMAKE_SOURCE_DIR}/libs/libraylib_windows.a")
    target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE
            ${RAYLIB_LIB}
            winmm
            gdi32
            opengl32
            shell32
    )
elseif(UNIX)
    # Linux settings
    set(RAYLIB_LIB "${CMAKE_SOURCE_DIR}/libs/libraylib_linux.a")
    target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE
            ${RAYLIB_LIB}
            m
            pthread
            dl
            rt
            X11
    )
endif()
