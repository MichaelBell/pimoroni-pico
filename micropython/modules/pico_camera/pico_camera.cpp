#include <cstdio>
#include "pico_camera.hpp"
#include "hardware/gpio.h"

#include "micropython/modules/util.hpp"

extern "C" {
#include "pico_camera.h"
#include "py/builtin.h"
#include "py/mpthread.h"

#define DEFAULT_BUFFER_LEN 4096

/* There can be only one camera */
static pimoroni::PicoCamera* camera = NULL;
static uint32_t* buffer = NULL;
static uint32_t buffer_len;

/* User might steal the SPI pins, so remember these and reset them across
 * functions that do work. */
gpio_function stored_fn;

static void save_spi_fn() {
    stored_fn = gpio_get_function(18);
    if (stored_fn != GPIO_FUNC_PIO1) {
        gpio_set_function(18, GPIO_FUNC_PIO1);
        gpio_set_function(19, GPIO_FUNC_PIO1);
        gpio_set_function(20, GPIO_FUNC_PIO1);
    }
}

static void restore_spi_fn() {
    gpio_set_function(18, stored_fn);
    gpio_set_function(19, stored_fn);
    gpio_set_function(20, stored_fn);
}

/***** Init module *****/
mp_obj_t pico_camera_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_buffer };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buffer, MP_ARG_OBJ, {.u_obj = nullptr} }
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (args[ARG_buffer].u_obj) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_buffer].u_obj, &bufinfo, MP_BUFFER_RW);
        if(bufinfo.len < 1024) {
            mp_raise_ValueError("Supplied buffer is too small!");
        }
        buffer_len = bufinfo.len;

        // Must align to 32-bit boundary and be a multiple of 16 long
        uintptr_t buf_addr = (uintptr_t)bufinfo.buf;
        buffer = (uint32_t *)((buf_addr + 3) & ~3u);
        buffer_len -= (uintptr_t)buffer - buf_addr;
        buffer_len &= ~0xfu;
    } else if (!buffer) {
        buffer = m_new(uint32_t, DEFAULT_BUFFER_LEN / 4);
        buffer_len = DEFAULT_BUFFER_LEN;
    }

    if (camera == NULL) {
        camera = new pimoroni::PicoCamera();
        save_spi_fn();
        camera->init(buffer, buffer_len);
        restore_spi_fn();
    }

    return mp_const_none;
}

mp_obj_t pico_camera_capture_image(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_slot };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_slot, MP_ARG_INT, {.u_int = 0}  },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int slot = args[ARG_slot].u_int;

    save_spi_fn();
    camera->capture_image(slot);
    restore_spi_fn();
    return mp_const_none;
}

mp_obj_t pico_camera_read_data(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_address, ARG_len, ARG_slot };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_address, MP_ARG_INT, {.u_int = 0}  },
        { MP_QSTR_len, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_slot, MP_ARG_INT, {.u_int = 0}  },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int slot = args[ARG_slot].u_int;
    uint32_t address = args[ARG_address].u_int;

    uint32_t len_to_read = args[ARG_len].u_int & ~0x3u;
    if (len_to_read == 0 || len_to_read > buffer_len) {
        len_to_read = buffer_len;
    }

    save_spi_fn();
    camera->read_data(slot, address, len_to_read, buffer);
    restore_spi_fn();

    return mp_obj_new_memoryview('B', len_to_read, buffer);
}

}
