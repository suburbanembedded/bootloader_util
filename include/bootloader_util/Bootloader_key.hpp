#pragma once

#include <array>

#include <cstdint>

class Bootloader_key
{
public:
	enum class Bootloader_ops : uint8_t
	{
		RUN_BOOTLDR,
		RUN_APP
	};

	Bootloader_key()
	{
		magic_sig.fill(0);
		bootloader_op = 0;
		crc32 = 0;
	}

	Bootloader_key(const Bootloader_ops op);

	static Bootloader_key get_key_boot();
	static Bootloader_key get_key_app();

	void to_addr(uint8_t* const addr) const;
	void from_addr(const uint8_t* addr);

	uint32_t calculate_crc() const;
	void update_crc();
	bool verify() const;
	
	void update_magic_sig();

	static constexpr std::array<uint8_t, 8> MAGIC_SIG_DEF = {0xCE, 0x10, 0x22, 0x8C, 0x92, 0xC6, 0xF8, 0xB9};

	std::array<uint8_t, 8> magic_sig;
	uint8_t bootloader_op;
	uint32_t crc32;
};
