#include "pico_camera.h"


/***** Module functions *****/
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(picocamera_init_obj, 0, pico_camera_init);

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(picocamera_capture_image_obj, 0, pico_camera_capture_image);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(picocamera_read_data_obj, 1, pico_camera_read_data);

/***** Globals Table *****/

STATIC const mp_rom_map_elem_t picocamera_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_picocamera) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&picocamera_init_obj) },

    { MP_ROM_QSTR(MP_QSTR_capture_image), MP_ROM_PTR(&picocamera_capture_image_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_data), MP_ROM_PTR(&picocamera_read_data_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mp_module_picocamera_globals, picocamera_globals_table);

/***** Module Definition *****/
const mp_obj_module_t picocamera_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_picocamera_globals,
};
#if MICROPY_VERSION <= 70144
MP_REGISTER_MODULE(MP_QSTR_picocamera, picocamera_user_cmodule, MODULE_PICOCAMERA_ENABLED);
#else
MP_REGISTER_MODULE(MP_QSTR_picocamera, picocamera_user_cmodule);
#endif
