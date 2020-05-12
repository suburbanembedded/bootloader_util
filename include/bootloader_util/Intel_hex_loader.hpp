/**
 * @brief Intel_hex_loader
 * @author Jacob Schloss <jacob@schloss.io>
 * @copyright Copyright (c) 2019 Jacob Schloss. All rights reserved.
 * @license Licensed under the 3-Clause BSD license. See LICENSE for details
*/

#pragma once

#include <cstdint>
#include <vector>
#include <string>

class IHEX_RECORD
{
public:
	enum class IHEX_RECORD_TYPE : uint8_t
	{
		REC_DATA						= 0x00,//data + 16bit addr
		REC_EOF							= 0x01,//end of file
		REC_EXTENDED_SEGMENT_ADDRESS	= 0x02,//16 addr bits, add to addr bits from data field
		REC_START_SEGMENT_ADDRESS		= 0x03,//CS:IP reg, 16 bit start address
		REC_EXTENDED_LINEAR_ADDRESS		= 0x04,//16 high addr bits, get low addr bits from data field
		REC_START_LINEAR_ADDRESS		= 0x05 //32 bit abs address to jump to
	};

	uint8_t byte_count;
	uint16_t address;
	IHEX_RECORD_TYPE record_type;
	std::vector<uint8_t> data;
	uint8_t checksum;

	bool verify_checksum() const;
	void update_checksum();

	static bool verify_checksum(const IHEX_RECORD& rec);
	static uint8_t calculate_checksum(const IHEX_RECORD& rec);

	bool to_string(std::string* const str);
	bool from_string(const std::string& str);
};

class Intel_hex_loader
{
public:

	Intel_hex_loader()
	{
		m_ext_lin_addr = 0;
		m_start_lin_addr = 0;
		m_has_start_lin_addr = false;

		m_eof = false;
	}

	bool process_line(const std::string& line);

	bool has_eof() const
	{
		return m_eof;
	}

	bool get_boot_addr(uint32_t* addr) const
	{
		if(!m_has_start_lin_addr)
		{
			return false;
		}
		
		*addr = m_start_lin_addr;
		return true;
	}

protected:

	bool handle_DATA(const IHEX_RECORD& rec);
	bool handle_EOF(const IHEX_RECORD& rec);
	bool handle_EXTENDED_SEGMENT_ADDRESS(const IHEX_RECORD& rec);
	bool handle_START_SEGMENT_ADDRESS(const IHEX_RECORD& rec);
	bool handle_EXTENDED_LINEAR_ADDRESS(const IHEX_RECORD& rec);
	bool handle_START_LINEAR_ADDRESS(const IHEX_RECORD& rec);

	uint32_t m_ext_lin_addr;

	bool     m_has_start_lin_addr;
	uint32_t m_start_lin_addr;

	bool m_eof;
};
