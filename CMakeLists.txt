
add_library(bootloader_util
	src/Intel_hex_loader.cpp
	src/Bootloader_key.cpp
)

target_include_directories(bootloader_util PUBLIC
	include/
)

target_link_libraries(bootloader_util
	common_util
)
