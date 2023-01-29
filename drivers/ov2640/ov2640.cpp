#include "ov2640.hpp"
#include "ov2640_init.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "ov2640.pio.h"

#include <cstdio>

namespace {
	alignas(1 << pimoroni::OV2640::RING_BUFFER_BITS) uint8_t ov2640_ring_buffer[1 << pimoroni::OV2640::RING_BUFFER_BITS];
}

namespace pimoroni {
	void OV2640::init(ImageSize size) {
		// XCLK generation (~15.62 MHz with RP2040 at 125MHz)
		gpio_set_function(pin_xclk, GPIO_FUNC_PWM);
		uint slice_num = pwm_gpio_to_slice_num(pin_xclk);
		// 8 cycles (0 to 7), 125 MHz / 8 = ~15.62 MHz wrap rate
		pwm_set_wrap(slice_num, 7);
		pwm_set_gpio_level(pin_xclk, 4);
		pwm_set_enabled(slice_num, true);

		// Initialise reset pin
		gpio_init(pin_resetb);
		gpio_set_dir(pin_resetb, GPIO_OUT);

		gpio_init(pin_vsync);
		gpio_set_dir(pin_vsync, GPIO_IN);

		// Reset camera, and give it some time to wake back up
		gpio_put(pin_resetb, 0);
		sleep_ms(100);
		gpio_put(pin_resetb, 1);
		sleep_ms(100);

		// Initialise the camera itself over SCCB
		regs_write(ov2640_svga);
		//ov2640_regs_write(config, ov2640_uxga_cif);

		// TODO: Support other sizes
		current_size = SIZE_1600x1200;

		// Set RGB565 output mode
		i2c->reg_write_uint8(OV2640_I2C_ADDRESS, 0xff, 0x00);
		i2c->reg_write_uint8(OV2640_I2C_ADDRESS, 0xDA, 0x09);

		// Enable image RX PIO
		uint offset = pio_add_program(pio, &ov2640_program);
		pio_sm = pio_claim_unused_sm(pio, true);
		ov2640_program_init(pio, pio_sm, offset, pin_y2_pio_base);

		// Claim DMA channel
		dma_channel = dma_claim_unused_channel(true);
	}

	uint32_t OV2640::get_image_len_in_bytes() const {
		switch (current_size) {
			case SIZE_1600x1200: return 1600 * 1200 * 2;
			case SIZE_800x600: return 800 * 600 * 2;
			case SIZE_352x288: return 352 * 288 * 2;
			default: return 0;
		}
	}

	uint32_t OV2640::start_capture() {
		dma_channel_config c = dma_channel_get_default_config(dma_channel);
		channel_config_set_read_increment(&c, false);
		channel_config_set_write_increment(&c, true);
		channel_config_set_dreq(&c, pio_get_dreq(pio, pio_sm, false));
		channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
		channel_config_set_bswap(&c, true);
		channel_config_set_ring(&c, true, OV2640::RING_BUFFER_BITS);
		
		dma_channel_configure(
			dma_channel, &c,
			ov2640_ring_buffer,
			&pio->rxf[pio_sm],
			get_image_len_in_bytes() >> 2,
			false
		);

		// Wait for vsync rising edge to start frame
		while (gpio_get(pin_vsync) == true);
		while (gpio_get(pin_vsync) == false);
		pio_sm_clear_fifos(pio, pio_sm);

		dma_channel_start(dma_channel);

		return get_image_len_in_bytes() >> 2;
	}

	void* OV2640::get_ring_buffer() const {
		return ov2640_ring_buffer;
	}

	void OV2640::regs_write(const uint8_t (*regs_list)[2]) {
		while (1) {
			uint8_t reg = (*regs_list)[0];
			uint8_t value = (*regs_list)[1];

			if (reg == 0x00 && value == 0x00) {
				break;
			}

			i2c->reg_write_uint8(OV2640_I2C_ADDRESS, reg, value);

			regs_list++;
		}
	}
}


