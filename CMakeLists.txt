
add_library(bootloader_key
	src/Bootloader_key.cpp
)

target_include_directories(bootloader_key PUBLIC
	include/
)

target_link_libraries(bootloader_key
	crc
)

add_library(bootloader_util
	src/Intel_hex_loader.cpp
)

target_include_directories(bootloader_util PUBLIC
	include/
)

target_link_libraries(bootloader_util
	bootloader_key
	common_util
	crc
)

if(DEFINED Doxygen::doxygen)
	doxygen_add_docs(bootloader_util_docs
		include/
		src/
	)
endif()
