#include "aps6404.hpp"
#include "hardware/dma.h"
#include "pico/stdlib.h"
#include "aps6404.pio.h"

namespace pimoroni {
	void APS6404::init() {
		// Initialize data pins
		for (int i = 0; i < 4; ++i) {
			gpio_init(pin_d0 + i);
			gpio_disable_pulls(pin_d0 + i);
		}

		uint offset = pio_add_program(pio, &sram_reset_program);
		pio_sm = pio_claim_unused_sm(pio, true);
		aps6404_reset_program_init(pio, pio_sm, offset, pin_csn, pin_d0);

		sleep_us(200);
		pio_sm_put_blocking(pio, pio_sm, 0x00000007u);
		pio_sm_put_blocking(pio, pio_sm, 0x66000000u);
		pio_sm_put_blocking(pio, pio_sm, 0x00000007u);
		pio_sm_put_blocking(pio, pio_sm, 0x99000000u);
		sleep_us(300);
		pio_sm_set_enabled(pio, pio_sm, false);

		pio_remove_program(pio, &sram_reset_program, offset);

		offset = pio_add_program(pio, &sram_program);
		//printf("SRAM program loaded to PIO at offset %d\n", offset);
		aps6404_program_init(pio, pio_sm, offset, pin_csn, pin_d0);

		// Claim DMA channel
		dma_channel = dma_claim_unused_channel(true);
	}

	void APS6404::write(uint32_t addr, uint32_t* data, uint32_t len_in_words) {
		wait_for_finish_blocking();

		dma_channel_config c = dma_channel_get_default_config(dma_channel);
		channel_config_set_read_increment(&c, true);
		channel_config_set_write_increment(&c, false);
		channel_config_set_dreq(&c, pio_get_dreq(pio, pio_sm, true));
		channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
		
		dma_channel_configure(
			dma_channel, &c,
			&pio->txf[pio_sm],
			data,
			len_in_words,
			false
		);

		pio_sm_put_blocking(pio, pio_sm, 0x80000000u | ((len_in_words * 8) - 1 + 6));
		pio_sm_put_blocking(pio, pio_sm, 0x38000000u | addr);

		dma_channel_start(dma_channel);
	}

	void APS6404::read(uint32_t addr, uint32_t* read_buf, uint32_t len_in_words) {
		wait_for_finish_blocking();

		dma_channel_config c = dma_channel_get_default_config(dma_channel);
		channel_config_set_read_increment(&c, false);
		channel_config_set_write_increment(&c, true);
		channel_config_set_dreq(&c, pio_get_dreq(pio, pio_sm, false));
		channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
		
		dma_channel_configure(
			dma_channel, &c,
			read_buf,
			&pio->rxf[pio_sm],
			len_in_words,
			true
		);

		pio_sm_put_blocking(pio, pio_sm, (len_in_words * 32) - 1);
		pio_sm_put_blocking(pio, pio_sm, 0x0b000000u | addr);
	}

	void APS6404::read_blocking(uint32_t addr, uint32_t* read_buf, uint32_t len_in_words) {
		while (len_in_words > 0) {
			uint32_t len = len_in_words;
			if (len > (PAGE_SIZE >> 2)) len = PAGE_SIZE >> 2;
			read(addr, read_buf, len);

			addr += len << 2;
			read_buf += len;
			len_in_words -= len;
		}

		wait_for_finish_blocking();
	}

	void APS6404::wait_for_finish_blocking() {
		dma_channel_wait_for_finish_blocking(dma_channel);
	}

	void APS6404::write_from_ring_buffer(uint32_t addr, void* ring_buffer, uint32_t len_in_words, uint ring_bits, uint source_dma_channel) {
		constexpr int words_per_transfer = 256;

		dma_channel_config c = dma_channel_get_default_config(dma_channel);
		channel_config_set_read_increment(&c, true);
		channel_config_set_write_increment(&c, false);
		channel_config_set_dreq(&c, pio_get_dreq(pio, pio_sm, true));
		channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
		channel_config_set_ring(&c, false, ring_bits);
		
		dma_channel_configure(
			dma_channel, &c,
			&pio->txf[pio_sm],
			ring_buffer,
			words_per_transfer,
			false
		);

		uint32_t write_cmd_and_addr = 0x38000000u | addr;
		uint32_t transfer_len_for_pio = 0x80000005u + (words_per_transfer << 3);
		int next_transfer_threshold = len_in_words - words_per_transfer;
		while (true) {
			while (dma_hw->ch[source_dma_channel].transfer_count > uint32_t(next_transfer_threshold));

			pio_sm_put_blocking(pio, pio_sm, transfer_len_for_pio);
			pio_sm_put_blocking(pio, pio_sm, write_cmd_and_addr);
			dma_channel_start(dma_channel);
			write_cmd_and_addr += words_per_transfer << 2;

			dma_channel_wait_for_finish_blocking(dma_channel);

			if (next_transfer_threshold == 0) break;
			else next_transfer_threshold -= words_per_transfer;

			if (next_transfer_threshold < 0) {
				const int final_transfer_len = next_transfer_threshold + words_per_transfer;
				transfer_len_for_pio = 0x80000005u + (final_transfer_len << 3);
				dma_hw->ch[dma_channel].transfer_count = final_transfer_len;
				
				next_transfer_threshold = 0;
			}
		}
	}
}

