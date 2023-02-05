// Include MicroPython API.
#include "py/runtime.h"
#include "py/objstr.h"

// Declare the functions we'll make available in Python
extern mp_obj_t pico_camera_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);

extern mp_obj_t pico_camera_capture_image(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);
extern mp_obj_t pico_camera_read_data(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);
