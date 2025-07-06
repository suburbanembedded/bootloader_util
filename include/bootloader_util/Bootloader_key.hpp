/**
 * @brief Bootloader_key
 * @author Jacob Schloss <jacob@schloss.io>
 * @copyright Copyright (c) 2018 Jacob Schloss. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/

#pragma once

#include <array>

#include <cstdint>

class Bootloader_key
{
public:
	enum class Bootloader_ops : uint32_t
	{
		RUN_BOOTLDR, // Stay in bootloader mode
		LOAD_APP,    // Load app into AXI sram, set op to RUN_APP and reset
		RUN_APP      // Run app
	};

	Bootloader_key()
	{
		magic_sig.fill(0);
		app_md5.fill(0);
		bootloader_op = 0;
		crc32 = 0;
	}

	Bootloader_key(const Bootloader_ops op);
	Bootloader_key(const Bootloader_ops op, const std::array<uint8_t, 16>& md5);

	static Bootloader_key get_key_boot();
	static Bootloader_key get_key_app();

	void to_addr(uint8_t volatile* const addr) const;
	void from_addr(uint8_t volatile const* const addr);

	uint32_t calculate_crc() const;
	void update_crc();
	bool verify() const;
	
	void update_magic_sig();

	static constexpr std::array<uint8_t, 8> MAGIC_SIG_DEF = {0xCE, 0x10, 0x22, 0x8C, 0x92, 0xC6, 0xF8, 0xB9};

	std::array<uint8_t, 8> magic_sig;
	std::array<uint8_t, 16> app_md5;
	uint32_t bootloader_op;
	uint32_t crc32;
};
