file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(OgreResourceEditor ${EDITOR_SOURCES})

# Mirror the folder layout in the Visual Studio solution explorer.
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${EDITOR_SOURCES})

target_include_directories(OgreResourceEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(OgreResourceEditor PRIVATE OgreMain OgreBites OgreOverlay)
target_compile_definitions(OgreResourceEditor PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
set_target_properties(OgreResourceEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

add_custom_command(TARGET OgreResourceEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_BINARY_DIR}/third_party/ogre/bin/$<CONFIG>"
        "$<TARGET_FILE_DIR:OgreResourceEditor>"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:OgreResourceEditor>/fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/third_party/imgui/misc/fonts/Roboto-Medium.ttf"
        "$<TARGET_FILE_DIR:OgreResourceEditor>/fonts/Roboto-Medium.ttf"
    COMMENT "Copying OGRE runtime files and editor fonts beside OgreResourceEditor")

if(MSVC)
    target_sources(OgreResourceEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/platform/OgreResourceEditor.manifest")
    target_compile_options(OgreResourceEditor PRIVATE /W4 /permissive- /Zc:__cplusplus)
endif()
