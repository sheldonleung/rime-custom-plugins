project(rime-custom)
cmake_minimum_required(VERSION 3.10)

aux_source_directory(src custom_src)

add_library(rime-custom-objs OBJECT ${custom_src})
if(BUILD_SHARED_LIBS)
  set_target_properties(rime-custom-objs
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON)
endif()

set(plugin_name rime-custom PARENT_SCOPE)
set(plugin_objs $<TARGET_OBJECTS:rime-custom-objs> PARENT_SCOPE)
set(plugin_deps ${rime_library} PARENT_SCOPE)
set(plugin_modules "custom" PARENT_SCOPE)
