#pragma once

#include "common/pimoroni_i2c.hpp"
#include "hardware/pio.h"

namespace pimoroni {
	// Driver for the OV2640 camera module
	//
	// Because captured images are normally much larger than the available RAM on a RP2040,
	// the normal mode of operation is for this module to capture data to a ring buffer
	// and for another module (e.g. a PSRAM) to copy the data out of the ring buffer as
	// it is captured.
	class OV2640 {
		public:
			static constexpr uint8_t OV2640_I2C_ADDRESS = 0x30;

			// Ring buffer of size 8kB.
			static constexpr uint32_t RING_BUFFER_BITS = 13;

			enum ImageSize {
				SIZE_1600x1200,
				SIZE_800x600,
				SIZE_352x288,
			};

			OV2640() : OV2640(new I2C()) {}
			OV2640(I2C* i2c, uint pin_resetb = 8, uint pin_xclk = 0, uint pin_vsync = 28, uint pin_data2 = 9, PIO pio = pio1)
			    : i2c(i2c) 
				, pin_resetb(pin_resetb)
				, pin_xclk(pin_xclk)
				, pin_vsync(pin_vsync)
				, pin_y2_pio_base(pin_data2)
				, pio(pio)
			{}

			void init(ImageSize size);
			void set_image_size(ImageSize size);

			uint32_t get_image_len_in_bytes() const;

			// Wait for the next frame to begin and start reading the image to the ring buffer
			// by DMA.  Calling code must be ready to store the data written to the ring buffer
			// immediately after this function returns.
			// Returns the number of words that will be captured.
			uint32_t start_capture();

			// Get the DMA channel used for the capture.  This channel will need to be
			// monitored after a call to start_capture in order to extract the data as it 
			// is written.
			uint get_dma_channel() const { return dma_channel; }

			// Get a pointer to the ring buffer
			void* get_ring_buffer() const;

		private:
			void regs_write(const uint8_t (*regs_list)[2]);

			I2C *i2c;

			uint pin_resetb;
			uint pin_xclk;
			uint pin_vsync;
			uint pin_y2_pio_base;

			PIO pio;
			uint pio_sm;

			uint dma_channel;

			ImageSize current_size;
	};
}
