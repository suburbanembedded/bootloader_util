/**
 * @brief Intel_hex_loader
 * @author Jacob Schloss <jacob@schloss.io>
 * @copyright Copyright (c) 2019 Jacob Schloss. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/

#include "bootloader_util/Intel_hex_loader.hpp"

#include "common_util/Byte_util.hpp"

#include <algorithm>

bool IHEX_RECORD::verify_checksum(const IHEX_RECORD& rec)
{
	return calculate_checksum(rec) == rec.checksum;
}
uint8_t IHEX_RECORD::calculate_checksum(const IHEX_RECORD& rec)
{
	uint8_t checksum = 0;

	checksum += rec.byte_count;
	checksum += Byte_util::get_b1(rec.address);
	checksum += Byte_util::get_b0(rec.address);
	checksum += static_cast<uint8_t>(rec.record_type);

	for(uint8_t x : rec.data)
	{
		checksum += x;
	}

	checksum = (~checksum) + 1;

	return checksum;
}

bool IHEX_RECORD::verify_checksum() const
{
	return IHEX_RECORD::verify_checksum(*this);
}
void IHEX_RECORD::update_checksum()
{
	checksum = IHEX_RECORD::calculate_checksum(*this);
}

bool IHEX_RECORD::to_string(std::string* const str)
{
	str->clear();
	str->reserve(1+2+4+2+data.size()*2+2);

	str->append(1, ':');

	std::array<char, 3> hex_str;

	Byte_util::u8_to_hex_str(byte_count, &hex_str);
	str->append(hex_str.data());

	Byte_util::u8_to_hex_str(Byte_util::get_b1(address), &hex_str);
	str->append(hex_str.data());

	Byte_util::u8_to_hex_str(Byte_util::get_b0(address), &hex_str);
	str->append(hex_str.data());

	Byte_util::u8_to_hex_str(static_cast<uint8_t>(record_type), &hex_str);
	str->append(hex_str.data());

	for(uint8_t x : data)
	{
		Byte_util::u8_to_hex_str(x, &hex_str);
		str->append(hex_str.data());
	}

	Byte_util::u8_to_hex_str(checksum, &hex_str);
	str->append(hex_str.data());

	return true;
}
bool IHEX_RECORD::from_string(const std::string& str)
{
	return from_string(str.data(), str.size());
}
bool IHEX_RECORD::from_string(const char* str, const size_t len)
{
	const char* ptr = str;

	if(ptr[0] != ':')
	{
		return false;
	}

	if(!Byte_util::hex_to_byte(ptr+1, &byte_count))
	{
		return false;
	}

	if(len < (1U+2U+4U+2U+byte_count*2U+2U))
	{
		return false;
	}

	if(!Byte_util::hex_to_u16(ptr+3, &address))
	{
		return false;
	}

	{
		uint8_t temp = 0;
		if(!Byte_util::hex_to_byte(ptr+7, &temp))
		{
			return false;
		}
		record_type = static_cast<IHEX_RECORD_TYPE>(temp);
	}

	data.resize(byte_count);
	for(size_t i = 0; i < data.size(); i++)
	{
		if(!Byte_util::hex_to_byte(ptr+9+2*i, data.data()+i))
		{
			return false;
		}
	}

	if(!Byte_util::hex_to_byte(ptr+9+2*byte_count, &checksum))
	{
		return false;
	}

	return true;
}

bool Intel_hex_loader::process_line(const char* line, const size_t len)
{
	IHEX_RECORD rec;

	if(!rec.from_string(line))
	{
		return false;
	}

	if(!rec.verify_checksum())
	{
		return false;	
	}

	bool ret = false;
	switch(rec.record_type)
	{
		case IHEX_RECORD::IHEX_RECORD_TYPE::REC_DATA:
		{
			ret = handle_DATA(rec);
			break;
		}
		case IHEX_RECORD::IHEX_RECORD_TYPE::REC_EOF:
		{
			ret = handle_EOF(rec);
			break;
		}
		case IHEX_RECORD::IHEX_RECORD_TYPE::REC_EXTENDED_SEGMENT_ADDRESS:
		{
			ret = handle_EXTENDED_SEGMENT_ADDRESS(rec);
			break;
		}
		case IHEX_RECORD::IHEX_RECORD_TYPE::REC_START_SEGMENT_ADDRESS:
		{
			ret = handle_START_SEGMENT_ADDRESS(rec);
			break;
		}
		case IHEX_RECORD::IHEX_RECORD_TYPE::REC_EXTENDED_LINEAR_ADDRESS:
		{
			ret = handle_EXTENDED_LINEAR_ADDRESS(rec);
			break;
		}
		case IHEX_RECORD::IHEX_RECORD_TYPE::REC_START_LINEAR_ADDRESS:
		{
			ret = handle_START_LINEAR_ADDRESS(rec);
			break;
		}
		default:
		{
			ret = false;
			break;
		}
	}

	return ret;
}

bool Intel_hex_loader::process_line(const std::string& line)
{
	return process_line(line.data(), line.size());
}

bool Intel_hex_loader::handle_DATA(const IHEX_RECORD& rec)
{
	volatile uint8_t* base_addr = reinterpret_cast<volatile uint8_t*>(m_ext_lin_addr + rec.address);

	std::copy_n(rec.data.data(), rec.byte_count, base_addr);

	return true;
}
bool Intel_hex_loader::handle_EOF(const IHEX_RECORD& rec)
{
	m_eof = true;
	return true;
}
bool Intel_hex_loader::handle_EXTENDED_SEGMENT_ADDRESS(const IHEX_RECORD& rec)
{
	//16 bit mode
	return false;
}
bool Intel_hex_loader::handle_START_SEGMENT_ADDRESS(const IHEX_RECORD& rec)
{
	//16 bit mode
	return false;
}
bool Intel_hex_loader::handle_EXTENDED_LINEAR_ADDRESS(const IHEX_RECORD& rec)
{
	if(rec.data.size() != 2)
	{
		return false;
	}

	m_ext_lin_addr = Byte_util::make_u32(
			rec.data[0],
			rec.data[1],
			0,
			0
		);

	return true;
}
bool Intel_hex_loader::handle_START_LINEAR_ADDRESS(const IHEX_RECORD& rec)
{
	if(rec.data.size() != 4)
	{
		return false;
	}

	m_start_lin_addr = Byte_util::make_u32(
			rec.data[0],
			rec.data[1],
			rec.data[2],
			rec.data[3]
		);
	m_has_start_lin_addr = true;

	return true;
}