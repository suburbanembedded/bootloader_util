#include "bootloader_util/Bootloader_key.hpp"

#include <algorithm>

void Bootloader_key::to_addr(uint8_t* const addr)
{
	volatile uint8_t* ptr = addr;
	std::copy_n(magic_sig.data(), magic_sig.size(), ptr);
	ptr += magic_sig.size();

	std::copy_n(&bootloader_op, 1, ptr);
	ptr += 1;

	std::copy_n(reinterpret_cast<uint8_t*>(&crc32), sizeof(crc32), ptr);
	ptr += sizeof(crc32);
}
void Bootloader_key::from_addr(const uint8_t* addr)
{
	volatile uint8_t const * ptr = addr;
	std::copy_n(ptr, magic_sig.size(), magic_sig.data());
	ptr += magic_sig.size();

	std::copy_n(ptr, 1, &bootloader_op);
	ptr += 1;

	std::copy_n(ptr, sizeof(crc32), reinterpret_cast<uint8_t*>(&crc32));
	ptr += sizeof(crc32);
}

uint32_t Bootloader_key::calculate_crc() const
{
	crc_32c::crc_t crc = crc_32c::crc_init();

	crc_32c::crc_update(crc, magic_sig.data(), magic_sig.size());
	crc_32c::crc_update(crc, &bootloader_op, 1);

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