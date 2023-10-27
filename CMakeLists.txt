cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME VolumetricRenderer)
project(${PROJECT_NAME} C CXX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -DVK_PROTOTYPES")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -DVK_PROTOTYPES")

set(CMAKE_CXX_STANDARD 20)
set(WARNINGS_AS_ERRORS OFF)
set(ENABLE_PVS_STUDIO OFF)

set(ENABLE_UBSan OFF)
set(ENABLE_ASAN OFF)
set(ENABLE_TSan OFF)
set(ENABLE_MSAN OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

# Wanna keep my switches compatible with VS
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions(_DEBUG)
else()
    add_compile_definitions(NDEBUG)
endif()

include(cmake/CompilerWarnings.cmake)

# Shortcuts
set(SOURCE_DIR  ${CMAKE_SOURCE_DIR}/source)
set(VENDOR_DIR  ${CMAKE_SOURCE_DIR}/vendor)

set(IMGUI_DIR ${VENDOR_DIR}/imgui)
set(GLFW_DIR ${VENDOR_DIR}/glfw)
set(LOGURU_DIR ${VENDOR_DIR}/loguru)

include_directories(${PROJECT_NAME} ${SOURCE_DIR})
include_directories(${PROJECT_NAME} ${SOURCE_DIR})
include_directories(${PROJECT_NAME} ${SOURCE_DIR})

# GLFW
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
add_subdirectory(${GLFW_DIR} bin EXCLUDE_FROM_ALL)
include_directories(${GLFW_DIR}/include)

# Dear ImGui
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)

# Loguru
set(LOGURU_WITH_STREAMS TRUE)
add_subdirectory(${LOGURU_DIR}) # defines target 'loguru::loguru'
include_directories(${LOGURU_DIR})

# Libraries
find_package(Vulkan REQUIRED)
set(LIBRARIES "glfw;Vulkan::Vulkan;loguru::loguru")

# Use vulkan headers from glfw:
include_directories(${GLFW_DIR}/deps)

file(GLOB_RECURSE sources ${SOURCE_DIR}/*)

add_executable(${PROJECT_NAME} ${sources} ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PUBLIC -DImTextureID=ImU64)


# Installation
install(PROGRAMS
        $<TARGET_FILE:${PROJECT_NAME}>
        DESTINATION bin)

set(ALL_TARGETS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

###
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
###

# Include CMake setup
include(cmake/main-config.cmake)
