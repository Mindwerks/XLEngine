cmake_minimum_required(VERSION 3.0.2)

project(XLEngine)

set(CMAKE_MODULE_PATH "${XLEngine_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)


include(CheckLibraryExists)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(DevIL REQUIRED)
find_package(Threads REQUIRED)
if(NOT WIN32)
    find_package(SDL2 REQUIRED)

    # Some systems need libdl for dlopen and friends.
    check_library_exists(dl dlopen "" HAVE_LIBDL)
endif(NOT WIN32)

add_subdirectory(thirdparty)

set(src_root ${XLEngine_SOURCE_DIR})

# Organize XL Engine sources into alphabetized groups.
# Can't use GLOB_RECURSE everywhere because some files are platform-specific.
file(GLOB_RECURSE fileformats_sources
    ${src_root}/fileformats/*.h*
    ${src_root}/fileformats/*.cpp)
file(GLOB_RECURSE math_sources
    ${src_root}/math/*.h*
    ${src_root}/math/*.cpp)
file(GLOB_RECURSE memory_sources
    ${src_root}/memory/*.h*
    ${src_root}/memory/*.cpp)
file(GLOB_RECURSE movieplayback_sources
    ${src_root}/movieplayback/*.h*
    ${src_root}/movieplayback/*.cpp)
file(GLOB_RECURSE networking_sources
    ${src_root}/networking/*.h*
    ${src_root}/networking/*.cpp)
set(os_sources
    ${src_root}/os/Clock.h
    ${src_root}/os/Input.h
    ${src_root}/os/Input.cpp
    ${src_root}/os/VirtualKeys.h)
file(GLOB_RECURSE plugin_framework_sources
    ${src_root}/plugin_framework/*.h*
    ${src_root}/plugin_framework/*.cpp)
file(GLOB_RECURSE procedural_sources
    ${src_root}/procedural/*.h*
    ${src_root}/procedural/*.cpp)
set(render_sources
    ${src_root}/render/Camera.h
    ${src_root}/render/Camera.cpp
    ${src_root}/render/CommandBuffer.h
    ${src_root}/render/CommandBuffer.cpp
    ${src_root}/render/Driver3D_IPlatform.h
    ${src_root}/render/Driver3D_OGL.h
    ${src_root}/render/Driver3D_OGL.cpp
    ${src_root}/render/Driver3D_Soft.h
    ${src_root}/render/Driver3D_Soft.cpp
    ${src_root}/render/Font.h
    ${src_root}/render/Font.cpp
    ${src_root}/render/FontManager.h
    ${src_root}/render/FontManager.cpp
    ${src_root}/render/IDriver3D.h
    ${src_root}/render/IDriver3D.cpp
    ${src_root}/render/ImageLoader.h
    ${src_root}/render/ImageLoader.cpp
    ${src_root}/render/IndexBuffer.h
    ${src_root}/render/IndexBuffer.cpp
    ${src_root}/render/Mesh.h
    ${src_root}/render/Mesh.cpp
    ${src_root}/render/MeshCache.h
    ${src_root}/render/MeshCache.cpp
    ${src_root}/render/RenderQue.h
    ${src_root}/render/RenderQue.cpp
    ${src_root}/render/TextureCache.h
    ${src_root}/render/TextureCache.cpp
    ${src_root}/render/triangleRasterizer/DrawScanline.h
    ${src_root}/render/triangleRasterizer/DrawScanline.cpp
    ${src_root}/render/triangleRasterizer/DrawScanline_Base.h
    ${src_root}/render/triangleRasterizer/TriangleRasterizer.h
    ${src_root}/render/triangleRasterizer/TriangleRasterizer.cpp
    ${src_root}/render/triangleRasterizer/VertexFormats.h
    ${src_root}/render/VertexBuffer.h
    ${src_root}/render/VertexBuffer.cpp)
set(root_sources
    ${src_root}/CommonTypes.h
    ${src_root}/Engine.h
    ${src_root}/Engine.cpp
    ${src_root}/EngineSettings.h
    ${src_root}/EngineSettings.cpp
    ${src_root}/PlatformDef.h
    ${src_root}/PluginAPI_Func.h)
file(GLOB_RECURSE scriptsystem_sources
    ${src_root}/scriptsystem/*.h*
    ${src_root}/scriptsystem/*.cpp)
set(threading_sources
    ${src_root}/threading/Mutex.h
    ${src_root}/threading/Semaphore.h
    ${src_root}/threading/Thread.h)
file(GLOB_RECURSE ui_sources
    ${src_root}/ui/*.h*
    ${src_root}/ui/*.cpp)
file(GLOB_RECURSE world_sources
    ${src_root}/world/*.h*
    ${src_root}/world/*.cpp)

set(xlengine_sources
    ${root_sources}
    ${fileformats_sources}
    ${math_sources}
    ${memory_sources}
    ${movieplayback_sources}
    ${networking_sources}
    ${os_sources}
    ${plugin_framework_sources}
    ${procedural_sources}
    ${render_sources}
    ${scriptsystem_sources}
    ${threading_sources}
    ${ui_sources}
    ${world_sources})

# Append platform-specific sources.
if(WIN32)
    file(GLOB_RECURSE os_windows_sources
        ${src_root}/os/Win/*.h*
        ${src_root}/os/Win/*.cpp)
    file(GLOB_RECURSE render_windows_sources
        ${src_root}/render/win/*.h*
        ${src_root}/render/win/*.cpp)
    file(GLOB_RECURSE threading_windows_sources
        ${src_root}/threading/Win32/*.h*
        ${src_root}/threading/Win32/*.cpp)
    set(xlengine_main_sources
        ${src_root}/win/main.h
        ${src_root}/win/main.cpp)
    set(xlengine_resources_sources
        ${src_root}/win/Resource.h
        ${src_root}/win/targetver.h
        "${src_root}/win/XL Engine.rc")

    set(os_sources ${os_sources} ${os_windows_sources})
    set(render_sources ${render_sources} ${render_windows_sources})
    set(threading_sources ${threading_sources} ${threading_windows_sources})
    set(xlengine_sources ${xlengine_sources}
        ${os_windows_sources}
        ${render_windows_sources}
        ${threading_windows_sources}
        ${xlengine_main_sources}
        ${xlengine_resources_sources})
else(WIN32)
    file(GLOB_RECURSE os_linux_sources
        ${src_root}/os/linux/*.h*
        ${src_root}/os/linux/*.cpp)
    file(GLOB_RECURSE render_linux_sources
        ${src_root}/render/linux/*.h*
        ${src_root}/render/linux/*.cpp)
    file(GLOB_RECURSE threading_posix_sources
        ${src_root}/threading/Posix/*.h*
        ${src_root}/threading/Posix/*.cpp)
    set(xlengine_main_sources
        ${src_root}/linux/main.cpp)

    set(os_sources ${os_sources} ${os_linux_sources})
    set(render_sources ${render_sources} ${render_linux_sources})
    set(threading_sources ${threading_sources} ${threading_posix_sources})
    set(xlengine_sources ${xlengine_sources}
        ${os_linux_sources}
        ${render_linux_sources}
        ${threading_posix_sources}
        ${xlengine_main_sources})
endif(WIN32)

# XL Engine Visual Studio filters.
source_group("fileformats" FILES ${fileformats_sources})
source_group("main" FILES ${xlengine_main_sources})
source_group("math" FILES ${math_sources})
source_group("memory" FILES ${memory_sources})
source_group("movieplayback" FILES ${movieplayback_sources})
source_group("networking" FILES ${networking_sources})
source_group("os" FILES ${os_sources})
source_group("plugin_framework" FILES ${plugin_framework_sources})
source_group("procedural" FILES ${procedural_sources})
source_group("resources" FILES ${xlengine_resources_sources})
source_group("render" FILES ${render_sources})
source_group("root" FILES ${root_sources})
source_group("scriptsystem" FILES ${scriptsystem_sources})
source_group("threading" FILES ${threading_sources})
source_group("ui" FILES ${ui_sources})
source_group("world" FILES ${world_sources})

# This path is included globally, since XLEngine and all the games use it.
include_directories(${XLEngine_SOURCE_DIR}/thirdparty/AngelCode/sdk/angelscript/include)

# Include/library directories.
# - TODO: find ENet. Currently has to be found and set manually.
# - TODO: Use enet64.lib on 64-bit.
set(xlengine_include_dirs
    ${GLEW_INCLUDE_DIR}
    ${IL_INCLUDE_DIR})
set(xlengine_libraries
    GLEW::GLEW
    ${OPENGL_LIBRARIES}
    ${IL_LIBRARIES}
    enet
    Angelscript
    Threads::Threads)

# Platform-specific includes/libraries.
if(WIN32)
    set(xlengine_libraries ${xlengine_libraries}
        winmm.lib
        ws2_32.lib)
else(WIN32)
    set(xlengine_include_dirs ${xlengine_include_dirs} ${SDL2_INCLUDE_DIR})
    set(xlengine_libraries ${xlengine_libraries} ${SDL2_LIBRARY})
    if(HAVE_LIBDL)
        set(xlengine_libraries ${xlengine_libraries} dl)
    endif(HAVE_LIBDL)
endif(WIN32)

add_executable(XLEngine ${xlengine_sources})
target_include_directories(XLEngine PRIVATE ${xlengine_include_dirs})
target_link_libraries(XLEngine PRIVATE ${xlengine_libraries})
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(XLEngine PROPERTIES INSTALL_RPATH "@executable_path")
elseif(NOT WIN32)
    set_target_properties(XLEngine PROPERTIES INSTALL_RPATH "$ORIGIN")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# XL Engine shared library sources.
set(bloodxl_sources
    ${src_root}/BloodXL/BloodXL_Game.cpp
    ${src_root}/BloodXL/BloodXL_main.cpp
    ${src_root}/BloodXL/BloodXL_Player.cpp)
set(daggerxl_sources
    ${src_root}/DaggerXL/DaggerXL_Game.cpp
    ${src_root}/DaggerXL/DaggerXL_main.cpp
    ${src_root}/DaggerXL/DaggerXL_Player.cpp
    ${src_root}/DaggerXL/Logic_Door.cpp
    ${src_root}/DaggerXL/Logic_NPC.cpp
    ${src_root}/DaggerXL/Logic_Obj_Action.cpp)
set(darkxl_sources
    ${src_root}/DarkXL/CutscenePlayer.cpp
    ${src_root}/DarkXL/DarkXL_Game.cpp
    ${src_root}/DarkXL/DarkXL_main.cpp)
set(outlawsxl_sources
    ${src_root}/OutlawsXL/OutlawsXL_Game.cpp
    ${src_root}/OutlawsXL/OutlawsXL_main.cpp
    ${src_root}/OutlawsXL/OutlawsXL_Player.cpp)

add_library(BloodXL SHARED ${bloodxl_sources})
add_library(DaggerXL SHARED ${daggerxl_sources})
add_library(DarkXL SHARED ${darkxl_sources})
add_library(OutlawsXL SHARED ${outlawsxl_sources})

set_target_properties(BloodXL PROPERTIES PREFIX "")
set_target_properties(DaggerXL PROPERTIES PREFIX "")
set_target_properties(DarkXL PROPERTIES PREFIX "")
set_target_properties(OutlawsXL PROPERTIES PREFIX "")

# Set project-specific XL_EXPORTS defines (case-sensitive). These must be
# set explicitly because the defaults are not the proper case.
set_target_properties(BloodXL PROPERTIES DEFINE_SYMBOL "BLOODXL_EXPORTS")
set_target_properties(DaggerXL PROPERTIES DEFINE_SYMBOL "DAGGERXL_EXPORTS")
set_target_properties(DarkXL PROPERTIES DEFINE_SYMBOL "DARKXL_EXPORTS")
set_target_properties(OutlawsXL PROPERTIES DEFINE_SYMBOL "OUTLAWSXL_EXPORTS")
