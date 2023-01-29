#include "pico/stdlib.h"
#include "pico_camera.hpp"

namespace pimoroni {
    void PicoCamera::init() {
        // Ensure SD card is deselected
        gpio_init(26);
        gpio_put(26, 1);
        gpio_set_dir(26, GPIO_OUT);

        ov2640.init(ImageSize::SIZE_1600x1200);
        aps6404.init();
    }

    void PicoCamera::capture_image(int slot) {
        const uint32_t addr = get_address_for_slot(slot);

        // TODO: Probably need to run this on core 1 to be reliable - should that happen here, or should the caller do it?
        const uint32_t image_len_in_words = ov2640.start_capture();
        aps6404.write_from_ring_buffer(addr, ov2640.get_ring_buffer(), image_len_in_words, OV2640::RING_BUFFER_BITS, ov2640.get_dma_channel());
    }

    int PicoCamera::count_image_slots() {
        const uint32_t image_len_in_pages = (get_image_len_in_bytes() + APS6404::PAGE_SIZE - 1) / APS6404::PAGE_SIZE;
        return APS6404::RAM_SIZE / (image_len_in_pages * APS6404::PAGE_SIZE);
    }

    void PicoCamera::read_data(int slot, uint32_t address, uint32_t len_in_bytes, uint32_t* buffer) {
        const uint32_t addr = get_address_for_slot(slot) + address;

        aps6404.read_blocking(addr, buffer, len_in_bytes >> 2);
    }

    uint32_t PicoCamera::get_address_for_slot(int slot) const {
        const uint32_t image_len_in_pages = (get_image_len_in_bytes() + APS6404::PAGE_SIZE - 1) / APS6404::PAGE_SIZE;
        return slot * image_len_in_pages * APS6404::PAGE_SIZE;
    }
}