/**
 * @brief Bootloader_key
 * @author Jacob Schloss <jacob@schloss.io>
 * @copyright Copyright (c) 2019 Jacob Schloss. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/

#include "bootloader_util/Bootloader_key.hpp"

#include "crc/crc_32c.hpp"

#include <algorithm>
#include <cstring>

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
	uint32_t volatile * ptr = reinterpret_cast<uint32_t volatile *>(addr);

	uint32_t tmp;
	for(size_t i = 0; i < 8/4; i++)
	{
		memcpy(&tmp, magic_sig.data()+i*4, 4);
		ptr[i] = tmp;
	}

	for(size_t i = 0; i < 16/4; i++)
	{
		memcpy(&tmp, app_md5.data()+i*4, 4);
		ptr[2+i] = tmp;
	}

	ptr[6] = app_length;
	ptr[7] = bootloader_op;
	ptr[8] = crc32;
}
void Bootloader_key::from_addr(uint8_t volatile const * const addr)
{
	uint32_t volatile const * ptr = reinterpret_cast<uint32_t volatile const *>(addr);

	uint32_t tmp;
	for(size_t i = 0; i < 8/4; i++)
	{
		tmp = ptr[i];
		memcpy(magic_sig.data()+i*4, &tmp, 4);
	}

	for(size_t i = 0; i < 16/4; i++)
	{
		tmp = ptr[2+i];
		memcpy(app_md5.data()+i*4, &tmp, 4);
	}

	app_length    = ptr[6];
	bootloader_op = ptr[7];
	crc32         = ptr[8];
}

uint32_t Bootloader_key::calculate_crc() const
{
	crc_32c::crc_t crc = crc_32c::crc_init();

	crc = crc_32c::crc_update(crc, magic_sig.data(), magic_sig.size());
	crc = crc_32c::crc_update(crc, app_md5.data(),   app_md5.size());
	crc = crc_32c::crc_update(crc, &app_length,      sizeof(app_length));
	crc = crc_32c::crc_update(crc, &bootloader_op,   sizeof(bootloader_op));

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