#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "drivers/aps6404/aps6404.hpp"
#include "drivers/ov2640/ov2640.hpp"
#include "common/pimoroni_i2c.hpp"

using namespace pimoroni;

int main() {
    stdio_init_all();

    sleep_ms(5000);

    APS6404 aps6404;
    I2C i2c(4, 5, 100000);
    OV2640 ov2640(&i2c);

    ov2640.init(OV2640::OV2640_1600x1200);
    aps6404.init();

    // Test reading/writing registers to the camera works
    i2c.reg_write_uint8(OV2640::OV2640_I2C_ADDRESS, 0xff, 0x01);
    uint8_t midh = i2c.reg_read_uint8(OV2640::OV2640_I2C_ADDRESS, 0x1c);
    uint8_t midl = i2c.reg_read_uint8(OV2640::OV2640_I2C_ADDRESS, 0x1d);

    // This should produce MIDH = 0x7f, MIDL = 0xa2
    printf("MIDH = 0x%02x, MIDL = 0x%02x\n", midh, midl);

    // Memory test
    uint32_t data[16];
    for (uint32_t i = 0; i < 16; ++i) {
        data[i] = 0x12345670u + i * 0x10101010u;
    }
    aps6404.write(0, data, 16);

    for (int i = 0; i < 16; ++i) {
        uint32_t read_data = 1;
        aps6404.read_blocking(i * 4, &read_data, 1);
        if (read_data != data[i]) {
            printf("RAM test failed: Wrote %lx, read back %lx\n", data[i], read_data);
        }
    }
    printf("RAM test complete\n");

    uint32_t image_len_in_words = ov2640.start_capture();
    aps6404.write_from_ring_buffer(0, ov2640.get_ring_buffer(), image_len_in_words, OV2640::RING_BUFFER_BITS, ov2640.get_dma_channel());
    printf("Image capture complete\n");

    while (1);

    return 0;
}
