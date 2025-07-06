/**
 * @brief Bootloader_key
 * @author Jacob Schloss <jacob@schloss.io>
 * @copyright Copyright (c) 2019 Jacob Schloss. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/

#include "bootloader_util/Bootloader_key.hpp"

#include "crc/crc_32c.hpp"

#include <algorithm>

constexpr std::array<uint8_t, 8> Bootloader_key::MAGIC_SIG_DEF;

Bootloader_key::Bootloader_key(const Bootloader_ops op)
{
	update_magic_sig();
	app_md5.fill(0);
	app_length = 0;
	bootloader_op = static_cast<uint8_t>(op);
	update_crc();
}

Bootloader_key::Bootloader_key(const Bootloader_ops op, const std::array<uint8_t, 16>& md5, const uint32_t length)
{
	update_magic_sig();
	app_md5 = md5;
	app_length = length;
	bootloader_op = static_cast<uint8_t>(op);
	update_crc();
}

Bootloader_key Bootloader_key::get_key_boot()
{
	return Bootloader_key(Bootloader_key::Bootloader_ops::RUN_BOOTLDR);
}
Bootloader_key Bootloader_key::get_key_app()
{
	return Bootloader_key(Bootloader_key::Bootloader_ops::RUN_APP);
}

void Bootloader_key::to_addr(uint8_t volatile * const addr) const
{
	uint8_t volatile * ptr = addr;

	std::copy_n(magic_sig.data(), magic_sig.size(), ptr);
	ptr += magic_sig.size();

	std::copy_n(app_md5.data(), app_md5.size(), ptr);
	ptr += app_md5.size();

	std::copy_n((uint8_t*)&app_length, sizeof(app_length), ptr);
	ptr += sizeof(app_length);

	std::copy_n((uint8_t*)&bootloader_op, sizeof(bootloader_op), ptr);
	ptr += sizeof(bootloader_op);

	std::copy_n((uint8_t*)&crc32, sizeof(crc32), ptr);
	ptr += sizeof(crc32);

	*ptr = bootloader_op;
}
void Bootloader_key::from_addr(uint8_t volatile const * const addr)
{
	uint8_t volatile const * ptr = addr;

	std::copy_n(ptr, magic_sig.size(), magic_sig.data());
	ptr += magic_sig.size();

	std::copy_n(ptr, app_md5.size(), app_md5.data());
	ptr += app_md5.size();

	std::copy_n(ptr, sizeof(app_length), (uint8_t*)&app_length);
	ptr += sizeof(app_length);

	std::copy_n(ptr, sizeof(bootloader_op), (uint8_t*)&bootloader_op);
	ptr += sizeof(bootloader_op);

	std::copy_n(ptr, sizeof(crc32), (uint8_t*)&crc32);
	ptr += sizeof(crc32);
}

uint32_t Bootloader_key::calculate_crc() const
{
	crc_32c::crc_t crc = crc_32c::crc_init();

	crc = crc_32c::crc_update(crc, magic_sig.data(), magic_sig.size());
	crc = crc_32c::crc_update(crc, app_md5.data(), app_md5.size());
	crc = crc_32c::crc_update(crc, &bootloader_op, sizeof(bootloader_op));

	crc = crc_32c::crc_finalize(crc);

	return crc;
}
void Bootloader_key::update_crc()
{
	crc32 = calculate_crc();
}
bool Bootloader_key::verify() const
{
	if(!std::equal(magic_sig.begin(), magic_sig.end(), MAGIC_SIG_DEF.begin()))
	{
		return false;
	}
	
	return crc32 == calculate_crc();
}

void Bootloader_key::update_magic_sig()
{
	magic_sig = MAGIC_SIG_DEF;
}