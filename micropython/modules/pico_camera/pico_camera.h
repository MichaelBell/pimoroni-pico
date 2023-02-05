// Include MicroPython API.
#include "py/runtime.h"
#include "py/objstr.h"

// These must match the enums in the OV2640 driver
enum PicoCameraImageSize {
    SIZE_1600x1200
};

enum PicoCameraImageMode {
    MODE_RGB565,
    MODE_YUYV
};

// Declare the functions we'll make available in Python
extern mp_obj_t pico_camera_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);

extern mp_obj_t pico_camera_get_image_len();
extern mp_obj_t pico_camera_set_image_size(mp_obj_t size);
extern mp_obj_t pico_camera_get_image_size();
extern mp_obj_t pico_camera_set_image_mode(mp_obj_t mode);
extern mp_obj_t pico_camera_get_image_mode();

extern mp_obj_t pico_camera_capture_image(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);
extern mp_obj_t pico_camera_read_data(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);
