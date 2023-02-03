set(MOD_NAME pico_camera)
string(TOUPPER ${MOD_NAME} MOD_NAME_UPPER)
add_library(usermod_${MOD_NAME} INTERFACE)

target_sources(usermod_${MOD_NAME} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/${MOD_NAME}.c
    ${CMAKE_CURRENT_LIST_DIR}/${MOD_NAME}.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../../libraries/pico_camera/pico_camera.cpp
)

target_include_directories(usermod_${MOD_NAME} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../../libraries/pico_camera/
)

target_compile_definitions(usermod_${MOD_NAME} INTERFACE
    MODULE_PICOCAMERA_ENABLED=1
)

target_link_libraries(usermod INTERFACE usermod_${MOD_NAME}
  pimoroni_i2c ov2640 aps6404)
