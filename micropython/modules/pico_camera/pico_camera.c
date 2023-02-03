#include "pico_camera.h"


/***** Methods *****/
MP_DEFINE_CONST_FUN_OBJ_1(PicoCamera___del___obj, PicoCamera___del__);

MP_DEFINE_CONST_FUN_OBJ_2(PicoCamera_capture_image_obj, PicoCamera_capture_image);
MP_DEFINE_CONST_FUN_OBJ_KW(PicoCamera_read_data_obj, 1, PicoCamera_read_data);

/***** Binding of Methods *****/
STATIC const mp_rom_map_elem_t PicoCamera_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&PicoCamera___del___obj) },
    { MP_ROM_QSTR(MP_QSTR_capture_image), MP_ROM_PTR(&PicoCamera_capture_image_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_data), MP_ROM_PTR(&PicoCamera_read_data_obj) },
};

STATIC MP_DEFINE_CONST_DICT(PicoCamera_locals_dict, PicoCamera_locals_dict_table);

/***** Class Definition *****/
#ifdef MP_DEFINE_CONST_OBJ_TYPE
MP_DEFINE_CONST_OBJ_TYPE(
    PicoCamera_type,
    MP_QSTR_PicoCamera,
    MP_TYPE_FLAG_NONE,
    make_new, PicoCamera_make_new,
    locals_dict, (mp_obj_dict_t*)&PicoCamera_locals_dict
);
#else
const mp_obj_type_t PicoCamera_type = {
    { &mp_type_type },
    .name = MP_QSTR_PicoCamera,
    .make_new = PicoCamera_make_new,
    .locals_dict = (mp_obj_dict_t*)&PicoCamera_locals_dict,
};
#endif

/***** Globals Table *****/

STATIC const mp_rom_map_elem_t pico_camera_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_picocamera) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_PicoCamera), (mp_obj_t)&PicoCamera_type },
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
