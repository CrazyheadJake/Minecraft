
cmake_minimum_required(VERSION 3.26.3)

# Unset these to statically link the C++ STL, important for if we are shipping the program 
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

# Support for MSVC runtime libraries
if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
    endif()
endif()

project(Minecraft)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Enable AddressSanitizer if supported
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fsanitize=address" HAS_ASAN_FLAG)

if(HAS_ASAN_FLAG AND ENABLE_ASAN)
    message("AddressSanitizer is enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
else()
    message("AddressSanitizer is disabled")
endif()

# we define MY_SOURCES to be a list of all the source files for my project
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${CMAKE_PROJECT_NAME} ${MY_SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# Path that will be set as CWD by the application
target_compile_definitions(${PROJECT_NAME} PUBLIC 
    ROOT_PATH="${CMAKE_SOURCE_DIR}"
)
# target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/") # This is useful to get an ASSETS_PATH in your IDE during development but you should comment this if you compile a release version and uncomment the next line
#target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/") # Uncomment this line to setup the ASSETS_PATH macro to the final assets directory when you share the project

# === Platform-specific configuration ===

if(WIN32)
    # Windows settings
    if (MSVC)
        set(RAYLIB_LIB "${CMAKE_SOURCE_DIR}/libs/libraylib_msvc.lib")
    else()
        set(RAYLIB_LIB "${CMAKE_SOURCE_DIR}/libs/libraylib_windows.a")
    endif()
    target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE
            ${RAYLIB_LIB}
            winmm
            gdi32
            opengl32
            shell32
            kernel32
            user32
    )
elseif(LINUX)
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
