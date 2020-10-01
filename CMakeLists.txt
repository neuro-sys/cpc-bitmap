cmake_minimum_required(VERSION 2.6.0)

project(cpc-bitmap)

add_executable(cpc-bitmap-sprite sprite.c ga.c crtc.c)
target_link_libraries(cpc-bitmap-sprite gif)

set_property(TARGET cpc-bitmap-sprite PROPERTY C_STANDARD 90)
set_property(TARGET cpc-bitmap-sprite PROPERTY C_EXTENSIONS false)

add_executable(cpc-bitmap-screen sprite.c ga.c crtc.c)
target_link_libraries(cpc-bitmap-screen gif)

set_property(TARGET cpc-bitmap-sprite PROPERTY C_STANDARD 90)
set_property(TARGET cpc-bitmap-sprite PROPERTY C_EXTENSIONS false)

add_executable(cpc-bitmap-convert-font convert-font.c)
target_link_libraries(cpc-bitmap-convert-font gif)

set_property(TARGET cpc-bitmap-convert-font PROPERTY C_STANDARD 90)
set_property(TARGET cpc-bitmap-convert-font PROPERTY C_EXTENSIONS false)

add_executable(cpc-bitmap-crtc crtc.c)
target_compile_definitions(cpc-bitmap-crtc PRIVATE -DMAIN)

set_property(TARGET cpc-bitmap-crtc PROPERTY C_STANDARD 90)
set_property(TARGET cpc-bitmap-crtc PROPERTY C_EXTENSIONS false)

