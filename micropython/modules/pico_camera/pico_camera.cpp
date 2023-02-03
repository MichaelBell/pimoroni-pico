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
    bool buf_allocated;
} _PicoCamera_obj_t;

/* There can be only one camera */
pimoroni::PicoCamera* _camera = NULL;

/***** Destructor ******/
mp_obj_t PicoCamera___del__(mp_obj_t self_in) {
    _PicoCamera_obj_t *self = MP_OBJ_TO_PTR2(self_in, _PicoCamera_obj_t);
    if (self->buf_allocated) {
        m_free(self->buf);
    }
    return mp_const_none;
}

/***** Constructor *****/
mp_obj_t PicoCamera_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_buffer };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buffer, MP_ARG_OBJ, {.u_obj = nullptr} }
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t *buffer = nullptr;
    uint32_t buffer_len;
    bool buffer_allocated = false;

    if (args[ARG_buffer].u_obj) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_buffer].u_obj, &bufinfo, MP_BUFFER_RW);
        if(bufinfo.len < 16) {
            mp_raise_ValueError("Supplied buffer is too small!");
        }
        buffer_len = bufinfo.len;

        // Must align to 32-bit boundary
        uintptr_t buf_addr = (uintptr_t)bufinfo.buf;
        buffer = (uint32_t *)((buf_addr + 3) & ~3u);
        buffer_len -= (uintptr_t)buffer - buf_addr;
        buffer_len &= ~3u;
    } else {
        buffer = m_new(uint32_t, DEFAULT_BUFFER_LEN / 4);
        buffer_len = DEFAULT_BUFFER_LEN;
        buffer_allocated = true;
    }

    _PicoCamera_obj_t* camera_obj = m_new_obj_with_finaliser(_PicoCamera_obj_t);
    camera_obj->base.type = &PicoCamera_type;
    camera_obj->buf = buffer;
    camera_obj->buf_len = buffer_len;
    camera_obj->buf_allocated = buffer_allocated;

    if (_camera == NULL) {
        _camera = camera_obj->camera = new pimoroni::PicoCamera();
        camera_obj->camera->init();
    }
    else {
        camera_obj->camera = _camera;
    }

    return MP_OBJ_FROM_PTR(camera_obj);
}

mp_obj_t PicoCamera_capture_image(mp_obj_t self_in, mp_obj_t slot) {
    _PicoCamera_obj_t *self = MP_OBJ_TO_PTR2(self_in, _PicoCamera_obj_t);
    self->camera->capture_image(mp_obj_get_int(slot));
    return mp_const_none;
}

mp_obj_t PicoCamera_read_data(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_slot, ARG_address, ARG_len };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_slot, MP_ARG_INT, {.u_int = 0}  },
        { MP_QSTR_address, MP_ARG_INT, {.u_int = 0}  },
        { MP_QSTR_len, MP_ARG_INT, {.u_int = 0} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _PicoCamera_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, _PicoCamera_obj_t);

    int slot = args[ARG_slot].u_int;
    uint32_t address = args[ARG_address].u_int;

    uint32_t len_to_read = args[ARG_len].u_int;
    if (len_to_read == 0 || len_to_read > self->buf_len) {
        len_to_read = self->buf_len;
    }

    self->camera->read_data(slot, address, len_to_read, (uint32_t*)self->buf);

    return mp_obj_new_memoryview('B', len_to_read, self->buf);
}

}
