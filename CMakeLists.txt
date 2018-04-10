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

find_package(DevIL REQUIRED)
find_package(ENet REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
if(NOT WIN32)
    find_package(SDL2 REQUIRED)

    # Some systems need libdl for dlopen and friends.
    check_library_exists(dl dlopen "" HAVE_LIBDL)
endif(NOT WIN32)

add_subdirectory(thirdparty)

set(src_root ${XLEngine_SOURCE_DIR})

# Organize XL Engine sources into alphabetized groups.
# Not using GLOB_RECURSE so file additions/deletions automatically trigger CMake.
set(fileformats_sources
    ${src_root}/fileformats/Archive.h
    ${src_root}/fileformats/ArchiveManager.h
    ${src_root}/fileformats/ArchiveManager.cpp
    ${src_root}/fileformats/ArchiveTypes.h
    ${src_root}/fileformats/ART_Reader.h
    ${src_root}/fileformats/ART_Reader.cpp
    ${src_root}/fileformats/BSA_Reader.h
    ${src_root}/fileformats/BSA_Reader.cpp
    ${src_root}/fileformats/CellLoader.h
    ${src_root}/fileformats/CellLoader_BloodMap.h
    ${src_root}/fileformats/CellLoader_BloodMap.cpp
    ${src_root}/fileformats/CellLoader_Daggerfall.h
    ${src_root}/fileformats/CellLoader_Daggerfall.cpp
    ${src_root}/fileformats/CellLoader_OutlawsMap.h
    ${src_root}/fileformats/CellLoader_OutlawsMap.cpp
    ${src_root}/fileformats/CellManager.h
    ${src_root}/fileformats/CellManager.cpp
    ${src_root}/fileformats/CellTypes.h
    ${src_root}/fileformats/DFFaceTex.h
    ${src_root}/fileformats/DFFaceTex.cpp
    ${src_root}/fileformats/GOB_Reader.h
    ${src_root}/fileformats/GOB_Reader.cpp
    ${src_root}/fileformats/LFD_Anim.h
    ${src_root}/fileformats/LFD_Anim.cpp
    ${src_root}/fileformats/LFD_Reader.h
    ${src_root}/fileformats/LFD_Reader.cpp
    ${src_root}/fileformats/Location_Daggerfall.h
    ${src_root}/fileformats/Location_Daggerfall.cpp
    ${src_root}/fileformats/MeshLoader_Daggerfall.h
    ${src_root}/fileformats/MeshLoader_Daggerfall.cpp
    ${src_root}/fileformats/Parser.h
    ${src_root}/fileformats/Parser.cpp
    ${src_root}/fileformats/RFF_Reader.h
    ${src_root}/fileformats/RFF_Reader.cpp
    ${src_root}/fileformats/SkyLoader.h
    ${src_root}/fileformats/SkyLoader_Daggerfall.h
    ${src_root}/fileformats/SkyLoader_Daggerfall.cpp
    ${src_root}/fileformats/TextureConv_ART.h
    ${src_root}/fileformats/TextureConv_ART.cpp
    ${src_root}/fileformats/TextureConv_IMG.h
    ${src_root}/fileformats/TextureConv_IMG.cpp
    ${src_root}/fileformats/TextureConv_PCX.h
    ${src_root}/fileformats/TextureConv_PCX.cpp
    ${src_root}/fileformats/TextureConverter.h
    ${src_root}/fileformats/TextureLoader.h
    ${src_root}/fileformats/TextureLoader.cpp
    ${src_root}/fileformats/TextureTypes.h)
set(math_sources
    ${src_root}/math/FixedPoint.h
    ${src_root}/math/FixedPoint.cpp
    ${src_root}/math/Math.h
    ${src_root}/math/Math.cpp
    ${src_root}/math/Matrix.h
    ${src_root}/math/Matrix.cpp
    ${src_root}/math/Plane.h
    ${src_root}/math/Plane.cpp
    ${src_root}/math/Vector2.h
    ${src_root}/math/Vector2.cpp
    ${src_root}/math/Vector3.h
    ${src_root}/math/Vector3.cpp
    ${src_root}/math/Vector4.h
    ${src_root}/math/Vector4.cpp)
set(memory_sources
    ${src_root}/memory/ScratchPad.h
    ${src_root}/memory/ScratchPad.cpp)
set(movieplayback_sources
    ${src_root}/movieplayback/LFD_Film.h
    ${src_root}/movieplayback/LFD_Film.cpp
    ${src_root}/movieplayback/MovieManager.h
    ${src_root}/movieplayback/MovieManager.cpp
    ${src_root}/movieplayback/MoviePlayer.h
    ${src_root}/movieplayback/MovieTypes.h)
set(networking_sources
    ${src_root}/networking/NetworkMgr.h
    ${src_root}/networking/NetworkMgr.cpp)
set(os_sources
    ${src_root}/os/Clock.h
    ${src_root}/os/Input.h
    ${src_root}/os/Input.cpp
    ${src_root}/os/VirtualKeys.h)
set(plugin_framework_sources
    ${src_root}/plugin_framework/DynamicLibrary.h
    ${src_root}/plugin_framework/DynamicLibrary.cpp
    ${src_root}/plugin_framework/plugin.h
    ${src_root}/plugin_framework/PluginManager.h
    ${src_root}/plugin_framework/PluginManager.cpp
    ${src_root}/plugin_framework/XLEngine_Plugin_API.h)
set(procedural_sources
    ${src_root}/procedural/Noise.h
    ${src_root}/procedural/Noise.cpp
    ${src_root}/procedural/ProceduralFunc.h
    ${src_root}/procedural/ProceduralFunc.cpp)
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
set(scriptsystem_sources
    ${src_root}/scriptsystem/scriptarray.h
    ${src_root}/scriptsystem/scriptarray.cpp
    ${src_root}/scriptsystem/scriptstdstring.h
    ${src_root}/scriptsystem/scriptstdstring.cpp
    ${src_root}/scriptsystem/ScriptSystem.h
    ${src_root}/scriptsystem/ScriptSystem.cpp)
set(threading_sources
    ${src_root}/threading/Mutex.h
    ${src_root}/threading/Semaphore.h
    ${src_root}/threading/Thread.h)
set(ui_sources
    ${src_root}/ui/Console.h
    ${src_root}/ui/Console.cpp
    ${src_root}/ui/UI_System.h
    ${src_root}/ui/UI_System.cpp
    ${src_root}/ui/XL_Console.h
    ${src_root}/ui/XL_Console.cpp)
set(world_sources
    ${src_root}/world/CollisionComponent.h
    ${src_root}/world/LevelFunc.h
    ${src_root}/world/LevelFunc.cpp
    ${src_root}/world/LevelFunc_Default.h
    ${src_root}/world/LevelFunc_Default.cpp
    ${src_root}/world/LevelFuncMgr.h
    ${src_root}/world/LevelFuncMgr.cpp
    ${src_root}/world/Logic.h
    ${src_root}/world/Logic.cpp
    ${src_root}/world/LogicDef.h
    ${src_root}/world/LogicManager.h
    ${src_root}/world/LogicManager.cpp
    ${src_root}/world/MeshCollision.h
    ${src_root}/world/MeshCollision.cpp
    ${src_root}/world/Object.h
    ${src_root}/world/Object.cpp
    ${src_root}/world/ObjectDef.h
    ${src_root}/world/ObjectManager.h
    ${src_root}/world/ObjectManager.cpp
    ${src_root}/world/OrientedSprite.h
    ${src_root}/world/OrientedSprite.cpp
    ${src_root}/world/RenderComponent.h
    ${src_root}/world/Sector.h
    ${src_root}/world/Sector.cpp
    ${src_root}/world/Sector_2_5D.h
    ${src_root}/world/Sector_2_5D.cpp
    ${src_root}/world/Sector_GeoBlock.h
    ${src_root}/world/Sector_GeoBlock.cpp
    ${src_root}/world/Sprite_ZAxis.h
    ${src_root}/world/Sprite_ZAxis.cpp
    ${src_root}/world/Terrain.h
    ${src_root}/world/Terrain.cpp
    ${src_root}/world/World.h
    ${src_root}/world/World.cpp
    ${src_root}/world/WorldCell.h
    ${src_root}/world/WorldCell.cpp)

set(xlengine_sources
    ${fileformats_sources}
    ${math_sources}
    ${memory_sources}
    ${movieplayback_sources}
    ${networking_sources}
    ${os_sources}
    ${plugin_framework_sources}
    ${procedural_sources}
    ${render_sources}
    ${root_sources}
    ${scriptsystem_sources}
    ${threading_sources}
    ${ui_sources}
    ${world_sources})

# Append platform-specific sources.
if(WIN32)
    set(os_windows_sources
        ${src_root}/os/Win/Clock_Win.cpp
        ${src_root}/os/Win/KeyOSMapping_Win.h)
    set(render_windows_sources
        ${src_root}/render/win/Driver3D_OGL_Win.h
        ${src_root}/render/win/Driver3D_OGL_Win.cpp
        ${src_root}/render/win/wglext.h)
    set(threading_windows_sources
        ${src_root}/threading/Win32/Mutex.h
        ${src_root}/threading/Win32/Semaphore.h
        ${src_root}/threading/Win32/Thread.h
        ${src_root}/threading/Win32/Win32.h)
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
    set(os_linux_sources
        ${src_root}/os/linux/Clock_Linux.cpp
        ${src_root}/os/linux/KeyOSMapping_Linux.h)
    set(render_linux_sources
        ${src_root}/render/linux/Driver3D_OGL_Linux.h
        ${src_root}/render/linux/Driver3D_OGL_Linux.cpp)
    set(threading_posix_sources
        ${src_root}/threading/Posix/Mutex.h
        ${src_root}/threading/Posix/Semaphore.h
        ${src_root}/threading/Posix/Thread.h)
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
set(xlengine_include_dirs
    ${ENET_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIR}
    ${IL_INCLUDE_DIR})
set(xlengine_libraries
    angelscript
    ${ENET_LIBRARIES}
    GLEW::GLEW
    ${IL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    Threads::Threads)

# Platform-specific includes/libraries.
if(WIN32)
    set(xlengine_libraries ${xlengine_libraries}
        winmm
        ws2_32)
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
    ${src_root}/BloodXL/BloodXL_Game.h
    ${src_root}/BloodXL/BloodXL_Game.cpp
    ${src_root}/BloodXL/BloodXL_main.h
    ${src_root}/BloodXL/BloodXL_main.cpp
    ${src_root}/BloodXL/BloodXL_Player.h
    ${src_root}/BloodXL/BloodXL_Player.cpp
    ${src_root}/BloodXL/targetver.h)
set(daggerxl_sources
    ${src_root}/DaggerXL/DaggerXL_Game.h
    ${src_root}/DaggerXL/DaggerXL_Game.cpp
    ${src_root}/DaggerXL/DaggerXL_main.h
    ${src_root}/DaggerXL/DaggerXL_main.cpp
    ${src_root}/DaggerXL/DaggerXL_Player.h
    ${src_root}/DaggerXL/DaggerXL_Player.cpp
    ${src_root}/DaggerXL/Logic_Door.h
    ${src_root}/DaggerXL/Logic_Door.cpp
    ${src_root}/DaggerXL/Logic_NPC.h
    ${src_root}/DaggerXL/Logic_NPC.cpp
    ${src_root}/DaggerXL/Logic_Obj_Action.h
    ${src_root}/DaggerXL/Logic_Obj_Action.cpp
    ${src_root}/DaggerXL/targetver.h)
set(darkxl_sources
    ${src_root}/DarkXL/CutscenePlayer.h
    ${src_root}/DarkXL/CutscenePlayer.cpp
    ${src_root}/DarkXL/DarkXL_Game.h
    ${src_root}/DarkXL/DarkXL_Game.cpp
    ${src_root}/DarkXL/DarkXL_main.h
    ${src_root}/DarkXL/DarkXL_main.cpp
    ${src_root}/DarkXL/targetver.h)
set(outlawsxl_sources
    ${src_root}/OutlawsXL/OutlawsXL_Game.h
    ${src_root}/OutlawsXL/OutlawsXL_Game.cpp
    ${src_root}/OutlawsXL/OutlawsXL_main.h
    ${src_root}/OutlawsXL/OutlawsXL_main.cpp
    ${src_root}/OutlawsXL/OutlawsXL_Player.h
    ${src_root}/OutlawsXL/OutlawsXL_Player.cpp
    ${src_root}/OutlawsXL/targetver.h)

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
target_compile_definitions(BloodXL PRIVATE "BLOODXL_EXPORTS")
target_compile_definitions(DaggerXL PRIVATE "DAGGERXL_EXPORTS")
target_compile_definitions(DarkXL PRIVATE "DARKXL_EXPORTS")
target_compile_definitions(OutlawsXL PRIVATE "OUTLAWSXL_EXPORTS")
