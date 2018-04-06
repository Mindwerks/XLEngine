cmake_minimum_required(VERSION 3.0.2)

project(XLEngine)


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
find_package(X11)
find_package(Threads)

check_library_exists(dl dlopen "" HAVE_LIBDL)

set(sources
    movieplayback/LFD_Film.cpp
    movieplayback/MovieManager.cpp
    ui/UI_System.cpp
    ui/XL_Console.cpp
    ui/Console.cpp
    scriptsystem/ScriptSystem.cpp
    scriptsystem/scriptarray.cpp
    scriptsystem/scriptstdstring.cpp
    procedural/ProceduralFunc.cpp
    procedural/Noise.cpp
    os/Input.cpp
    EngineSettings.cpp
    plugin_framework/DynamicLibrary.cpp
    plugin_framework/PluginManager.cpp
    Engine.cpp
    math/Matrix.cpp
    math/Vector2.cpp
    math/Plane.cpp
    math/Vector4.cpp
    math/Vector3.cpp
    math/FixedPoint.cpp
    math/Math.cpp
    fileformats/TextureConv_IMG.cpp
    fileformats/LFD_Reader.cpp
    fileformats/ArchiveManager.cpp
    fileformats/LFD_Anim.cpp
    fileformats/CellManager.cpp
    fileformats/MeshLoader_Daggerfall.cpp
    fileformats/CellLoader_OutlawsMap.cpp
    fileformats/Location_Daggerfall.cpp
    fileformats/TextureConv_PCX.cpp
    fileformats/GOB_Reader.cpp
    fileformats/BSA_Reader.cpp
    fileformats/RFF_Reader.cpp
    fileformats/TextureLoader.cpp
    fileformats/SkyLoader_Daggerfall.cpp
    fileformats/ART_Reader.cpp
    fileformats/CellLoader_BloodMap.cpp
    fileformats/CellLoader_Daggerfall.cpp
    fileformats/DFFaceTex.cpp
    fileformats/TextureConv_ART.cpp
    fileformats/Parser.cpp
    world/LevelFunc_Default.cpp
    world/Terrain.cpp
    world/Sprite_ZAxis.cpp
    world/Logic.cpp
    world/ObjectManager.cpp
    world/World.cpp
    world/WorldCell.cpp
    world/MeshCollision.cpp
    world/Sector_2_5D.cpp
    world/OrientedSprite.cpp
    world/LogicManager.cpp
    world/Object.cpp
    world/Sector.cpp
    world/LevelFunc.cpp
    world/LevelFuncMgr.cpp
    world/Sector_GeoBlock.cpp
    render/CommandBuffer.cpp
    render/Font.cpp
    render/RenderQue.cpp
    render/FontManager.cpp
    render/MeshCache.cpp
    render/Driver3D_Soft.cpp
    render/ImageLoader.cpp
    render/Camera.cpp
    render/TextureCache.cpp
    render/Driver3D_OGL.cpp
    render/IDriver3D.cpp
    render/VertexBuffer.cpp
    render/triangleRasterizer/TriangleRasterizer.cpp
    render/triangleRasterizer/DrawScanline.cpp
    render/Mesh.cpp
    render/IndexBuffer.cpp
    networking/NetworkMgr.cpp
    memory/ScratchPad.cpp
)

if(WIN32)
    set(sources ${sources}
        render/win/Driver3D_OGL_Win.cpp
        win/main.cpp
        os/Win/Clock_Win.cpp
    )
else(WIN32)
    set(sources ${sources}
        render/linux/Driver3D_OGL_Linux.cpp
        os/linux/Clock_Linux.cpp
        linux/main.cpp
    )
endif(WIN32)

add_executable(XLEngine ${sources})
target_link_libraries(XLEngine
     GLEW::GLEW ${OPENGL_LIBRARIES} ${IL_LIBRARIES} enet angelscript
     Threads::Threads
)
if(X11_FOUND)
    target_link_libraries(XLEngine ${X11_LIBRARIES})
endif(X11_FOUND)
if(HAVE_LIBDL)
    target_link_libraries(XLEngine dl)
endif(HAVE_LIBDL)


set(darkxl_sources
    DarkXL/CutscenePlayer.cpp
    DarkXL/DarkXL_Game.cpp
    DarkXL/DarkXL_main.cpp
)
add_library(DarkXL SHARED ${darkxl_sources})
set_target_properties(DarkXL PROPERTIES PREFIX "")


set(daggerxl_sources
    DaggerXL/DaggerXL_Game.cpp
    DaggerXL/DaggerXL_main.cpp
    DaggerXL/DaggerXL_Player.cpp
    DaggerXL/Logic_Door.cpp
    DaggerXL/Logic_NPC.cpp
    DaggerXL/Logic_Obj_Action.cpp
)
add_library(DaggerXL SHARED ${daggerxl_sources})
set_target_properties(DaggerXL PROPERTIES PREFIX "")


set(bloodxl_sources
    BloodXL/BloodXL_Game.cpp
    BloodXL/BloodXL_main.cpp
    BloodXL/BloodXL_Player.cpp
)
add_library(BloodXL SHARED ${bloodxl_sources})
set_target_properties(BloodXL PROPERTIES PREFIX "")


set(outlawsxl_sources
    OutlawsXL/OutlawsXL_Game.cpp
    OutlawsXL/OutlawsXL_main.cpp
    OutlawsXL/OutlawsXL_Player.cpp
)
add_library(OutlawsXL SHARED ${outlawsxl_sources})
set_target_properties(OutlawsXL PROPERTIES PREFIX "")
