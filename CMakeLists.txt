project(rime-custom-plugins)
cmake_minimum_required(VERSION 3.10)

aux_source_directory(src custom-plugins_src)

add_library(rime-custom-plugins-objs OBJECT ${custom-plugins_src})
if(BUILD_SHARED_LIBS)
  set_target_properties(rime-custom-plugins-objs
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON)
endif()

set(plugin_name rime-custom-plugins PARENT_SCOPE)
set(plugin_objs $<TARGET_OBJECTS:rime-custom-plugins-objs> PARENT_SCOPE)
set(plugin_deps ${rime_library} PARENT_SCOPE)
set(plugin_modules "custom-plugins" PARENT_SCOPE)
