#pragma once

#include <stdint.h>
#include "hardware/pio.h"

namespace pimoroni {
	class APS6404 {
		public:
			APS6404(uint pin_csn = 17, uint pin_d0 = 19, PIO pio = pio1)
				: pin_csn(pin_csn)
				, pin_d0(pin_d0)
				, pio(pio)
			{}

			void init();

			// Start a write, this completes asynchronously, this function only blocks if another 
			// transfer is already in progress
			void write(uint32_t addr, uint32_t* data, uint32_t len_in_words);

			// Start a read, this completes asynchronously, this function only blocks if another 
			// transfer is already in progress
			void read(uint32_t addr, uint32_t* read_buf, uint32_t len_in_words);

			// Read and block until complete
			void read_blocking(uint32_t addr, uint32_t* read_buf, uint32_t len_in_words) {
				read(addr, read_buf, len_in_words);
				wait_for_finish_blocking();
			}

			// Block until any outstanding read or write completes
			void wait_for_finish_blocking();

			// Write data that is being transferred into a ring buffer by another DMA channel
			void write_from_ring_buffer(uint32_t addr, void* ring_buffer, uint32_t len_in_words, uint ring_bits, uint source_dma_channel);

		private:
			uint pin_csn;  // CSn, SCK must be next pin after CSn
			uint pin_d0;   // D0, D1, D2, D3 must be consecutive

			PIO pio;
			uint pio_sm;

			uint dma_channel;
	};
}
