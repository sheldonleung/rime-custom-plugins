project(rime-wanxiang)
cmake_minimum_required(VERSION 3.10)

aux_source_directory(src wanxiang_src)

add_library(rime-wanxiang-objs OBJECT ${wanxiang_src})
if(BUILD_SHARED_LIBS)
  set_target_properties(rime-wanxiang-objs
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON)
endif()

set(plugin_name rime-wanxiang PARENT_SCOPE)
set(plugin_objs $<TARGET_OBJECTS:rime-wanxiang-objs> PARENT_SCOPE)
set(plugin_deps ${rime_library} PARENT_SCOPE)
set(plugin_modules "wanxiang" PARENT_SCOPE)
