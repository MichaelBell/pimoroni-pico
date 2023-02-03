#include <cstdio>
#include "pico_camera.hpp"

#include "micropython/modules/util.hpp"

extern "C" {
#include "pico_camera.h"
#include "py/builtin.h"
#include "py/mpthread.h"

#define DEFAULT_BUFFER_LEN 1024

/***** Variables Struct *****/
typedef struct _PicoCamera_obj_t {
    mp_obj_base_t base;
    pimoroni::PicoCamera* camera;
    void *buf;
    uint32_t buf_len;
} _PicoCamera_obj_t;

_PicoCamera_obj_t *camera_obj;


/***** Destructor ******/
mp_obj_t PicoCamera___del__(mp_obj_t self_in) {
    _PicoCamera_obj_t *self = MP_OBJ_TO_PTR2(self_in, _PicoCamera_obj_t);
    delete self->camera;
    return mp_const_none;
}

/***** Constructor *****/
mp_obj_t Badger2040_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_buffer };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buffer, MP_ARG_OBJ, {.u_obj = nullptr} }
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint8_t *buffer = nullptr;
    uint32_t buffer_len;

    if (args[ARG_buffer].u_obj) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_buffer].u_obj, &bufinfo, MP_BUFFER_RW);
        buffer = (uint8_t *)bufinfo.buf;
        if(bufinfo.len < 16) {
            mp_raise_ValueError("Supplied buffer is too small!");
        }
        buffer_len = bufinfo.len;
    } else {
        buffer = m_new(uint32_t, DEFAULT_BUFFER_LEN / 4);
        buffer_len = DEFAULT_BUFFER_LEN;
    }

    camera_obj = m_new_obj_with_finaliser(_PicoCamera_obj_t);
    camera_obj->base.type = &PicoCamera_type;
    camera_obj->buf = buffer;
    camera_obj->buf_len = buffer_len;

    camera_obj->camera = new pimoroni::PicoCamera();
    camera_obj->camera->init();

    return MP_OBJ_FROM_PTR(camera_obj);
}

mp_obj_t PicoCamera_capture_image(mp_obj_t self_in, mp_obj_t slot) {
    _PicoCamera_obj_t *self = MP_OBJ_TO_PTR2(self_in, _PicoCamera_obj_t);
    self->camera->capture_image(mp_obj_get_int(slot));
    return mp_const_none;
}

mp_obj_t PicoCamera_read_data(mp_obj_t self_in, mp_obj_t slot, mp_obj_t address, mp_obj_t len) {
    _PicoCamera_obj_t *self = MP_OBJ_TO_PTR2(self_in, _PicoCamera_obj_t);

    int len_to_read = mp_obj_get_int(len);
    if (len_to_read > self->camera->buf_len) {
        len_to_read = self->camera->buf_len;
    }

    self->camera->read_data(mp_obj_get_int(slot), mp_obj_get_int(address), len_to_read, (uint32_t*)self->camera->buf);

    return mp_obj_new_memoryview('B', len_to_read, self->camera->buf);
}

}
