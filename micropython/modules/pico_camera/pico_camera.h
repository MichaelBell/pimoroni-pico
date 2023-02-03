// Include MicroPython API.
#include "py/runtime.h"
#include "py/objstr.h"

/***** Extern of Class Definition *****/
extern const mp_obj_type_t Badger2040_type;

/***** Extern of Class Methods *****/
extern mp_obj_t PicoCamera___del__(mp_obj_t self_in);
extern mp_obj_t PicoCamera_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args);

extern mp_obj_t PicoCamera_capture_image(mp_obj_t self_in, mp_obj_t slot);
extern mp_obj_t PicoCamera_read_data(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);