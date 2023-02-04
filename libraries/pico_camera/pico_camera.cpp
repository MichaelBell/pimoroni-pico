#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "pico_camera.hpp"

namespace pimoroni {
    void PicoCamera::init() {
        // Ensure SD card is deselected
        gpio_init(26);
        gpio_put(26, 1);
        gpio_set_dir(26, GPIO_OUT);

        ov2640.init(ImageSize::SIZE_1600x1200);
        aps6404.init();

        for (int i = 0; i < NUM_BUFFERS; ++i) {
            buffers[i] = (uint32_t*)malloc(APS6404::PAGE_SIZE);
        }
    }

    void PicoCamera::capture_image(int slot) {
        transfer_addr = get_address_for_slot(slot);

        ov2640.start_capture(buffers, NUM_BUFFERS, APS6404::PAGE_SIZE >> 2, [this](uint32_t* buffer) {
            aps6404.enqueue_write(transfer_addr, buffer, APS6404::PAGE_SIZE >> 2);
            transfer_addr += APS6404::PAGE_SIZE;
        });

        while (ov2640.is_capture_in_progress()) {
            __wfi();
        }
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