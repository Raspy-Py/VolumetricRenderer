cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME VolumetricRenderer)
project(${PROJECT_NAME} C CXX)

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

set(SOURCE  ${CMAKE_SOURCE_DIR}/source)
set(VENDOR  ${CMAKE_SOURCE_DIR}/vendor)
# Shortcuts
set(CORE    ${SOURCE}/Etna/Core)
set(VULKAN  ${CORE}/Vulkan)
set(LAYERS  ${SOURCE}/Etna/Layers)

add_executable(${PROJECT_NAME}
    # - Source
    # -- Files you might wanna look at
    ${SOURCE}/EtnaApplication.cpp
    ${SOURCE}/Etna/Core/EntryPoint.cpp
    ${CORE}/Etna.h         ${CORE}/Etna.cpp
    ${CORE}/Utils.h        ${CORE}/Utils.cpp
    ${CORE}/Clock.h        ${CORE}/Clock.cpp
    ${CORE}/Exception.h    ${CORE}/Exception.cpp
    ${CORE}/ImGuiTheme.h   ${CORE}/ImGuiTheme.cpp
    # -- Application layers
    ${LAYERS}/Canvas/Canvas.h     ${LAYERS}/Canvas/Canvas.cpp
    # -- Vulkan render subsystem
    ${VULKAN}/VulkanHeader.h
    ${VULKAN}/VulkanVertexLayout.h
    ${VULKAN}/VulkanAttachment.h      ${VULKAN}/VulkanAttachment.cpp
    ${VULKAN}/VulkanContext.h         ${VULKAN}/VulkanContext.cpp
    ${VULKAN}/VulkanCore.h            ${VULKAN}/VulkanCore.cpp
    ${VULKAN}/VulkanDebugMessenger.h  ${VULKAN}/VulkanDebugMessenger.cpp
    ${VULKAN}/VulkanDevice.h          ${VULKAN}/VulkanDevice.cpp
    ${VULKAN}/VulkanFragmentShader.h  ${VULKAN}/VulkanFragmentShader.cpp
    ${VULKAN}/VulkanIndexBuffer.h     ${VULKAN}/VulkanIndexBuffer.cpp
    ${VULKAN}/VulkanInstance.h        ${VULKAN}/VulkanInstance.cpp
    ${VULKAN}/VulkanPipeline.h        ${VULKAN}/VulkanPipeline.cpp
    ${VULKAN}/VulkanRenderer.h        ${VULKAN}/VulkanRenderer.cpp
    ${VULKAN}/VulkanRenderPass.h      ${VULKAN}/VulkanRenderPass.cpp
    ${VULKAN}/VulkanShader.h          ${VULKAN}/VulkanShader.cpp
    ${VULKAN}/VulkanSurface.h         ${VULKAN}/VulkanSurface.cpp
    ${VULKAN}/VulkanSwapchain.h       ${VULKAN}/VulkanSwapchain.cpp
    ${VULKAN}/VulkanUniformBuffer.h   ${VULKAN}/VulkanUniformBuffer.cpp
    ${VULKAN}/VulkanVertexBuffer.h    ${VULKAN}/VulkanVertexBuffer.cpp
    ${VULKAN}/VulkanVertexShader.h    ${VULKAN}/VulkanVertexShader.cpp
    ${VULKAN}/VulkanShader.h          ${VULKAN}/VulkanShader.cpp
    ${VULKAN}/VulkanShader.h          ${VULKAN}/VulkanShader.cpp
    # - Third-party
    # -- Loguru (logging utility)
    ${VENDOR}/loguru/loguru.hpp
    ${VENDOR}/loguru/loguru.cpp
    # -- ImGui  (GUI framework + Vulkan backend)
    ${VENDOR}/imgui/imconfig.h
    ${VENDOR}/imgui/imgui.h
    ${VENDOR}/imgui/imgui_impl_glfw.h
    ${VENDOR}/imgui/imgui_impl_vulkan.h
    ${VENDOR}/imgui/imgui_internal.h
    ${VENDOR}/imgui/imstb_rectpack.h
    ${VENDOR}/imgui/imstb_textedit.h
    ${VENDOR}/imgui/imstb_truetype.h
    ${VENDOR}/imgui/imgui_impl_glfw.cpp
    ${VENDOR}/imgui/imgui_impl_vulkan.cpp
    ${VENDOR}/imgui/imgui.cpp
    ${VENDOR}/imgui/imgui_demo.cpp
    ${VENDOR}/imgui/imgui_draw.cpp
    ${VENDOR}/imgui/imgui_tables.cpp
    ${VENDOR}/imgui/imgui_widgets.cpp
)


target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE})

##########################################################
# Vendor
##########################################################

set(GLFW_BUILD_DOCS OFF CACHE BOOL "F" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${Vulkan_INCLUDE_DIR}
    PUBLIC ${GLFW_INCLUDE_DIRS}
    PUBLIC ${VENDOR}
)

target_link_libraries(${PROJECT_NAME}
    ${Vulkan_LIBRARIES}
    glfw
)

##########################################################
# Fixed CMakeLists.txt part
##########################################################

INSTALL(PROGRAMS
    $<TARGET_FILE:${PROJECT_NAME}>
    DESTINATION bin)

set(ALL_TARGETS ${PROJECT_NAME})

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
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