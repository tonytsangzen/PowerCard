cmake_minimum_required(VERSION 3.23)

set (toolcahin_install_path "toolchain")
set (download_path "temp")

if (APPLE)
	set(toolchain_package "MRS_Toolchain_MAC_V160.zip")
	set(toolchain_zip "xpack-riscv-none-embed-gcc-8.2.0.zip")
	set(openocd_zip "openocd_x86_64.zip")
	set(unpack_cmd "unzip")
elseif (UNIX)
	set(unpack_cmd "tar xf")
	set(toolchain_package "MRS_Toolchain_Linux_x64_V1.60.tar.xz")
else ()
	message(FATAL_ERROR  "Unsupport build OS!")
endif ()

message(STATUS "Checking toolchain......")

if ( NOT EXISTS ${toolcahin_install_path}/xpack-riscv-none-embed-gcc-8.2.0)

	file(MAKE_DIRECTORY ${toolcahin_install_path})

	if ( NOT EXISTS ${download_path}/${toolchain_package})
	    message(status "Downloading ${toolchain_package}......")
	    file(DOWNLOAD "http://file.mounriver.com/tools/${toolchain_package}" "${download_path}/${toolchain_package}"
			TIMEOUT 600
			STATUS err
			SHOW_PROGRESS
		)
		if ( NOT err EQUAL 0 )
			MESSAGE(STATUS "Download failed, error: ${err}")
			MESSAGE(FATAL_ERROR "You can try downloading ${toolchain_package} manually")
		endif()
	endif()

    message(STATUS "Unpackage ${toolchain_package}......")
	execute_process(
		COMMAND ${unpack_cmd} -o  ${CMAKE_SOURCE_DIR}/${download_path}/${toolchain_package}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${download_path}
		RESULT_VARIABLE err
	)
	if( NOT err EQUAL 0)
		MESSAGE(STATUS "Unpackage ${toolchain_package} error: ${err}")
		MESSAGE(FATAL_ERROR	"You can try unpackage ${toolchain_package} manually")
	endif()
	

    message(STATUS "Unpackage ${toolchain_zip}......")
	execute_process(
		COMMAND ${unpack_cmd} -o ${CMAKE_SOURCE_DIR}/${download_path}/${toolchain_zip}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${toolcahin_install_path}
		RESULT_VARIABLE err
	)
	if( NOT err EQUAL 0)
		MESSAGE(STATUS "Unpackage ${toolchain_zip} error: ${err}")
		MESSAGE(FATAL_ERROR	"You can try unpackage ${toolchain_zip} manually")
	endif()

    message(STATUS "Unpackage ${openocd_zip}......")
	execute_process(
		COMMAND ${unpack_cmd} -o ${CMAKE_SOURCE_DIR}/${download_path}/${openocd_zip}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${toolcahin_install_path}
		RESULT_VARIABLE err
	)
	if( NOT err EQUAL 0)
		MESSAGE(STATUS "Unpackage ${openocd_zip} error: ${err}")
		MESSAGE(FATAL_ERROR	"You can try unpackage ${openocd_zip} manually")
	endif()
	
endif()

add_subdirectory(bootloader)
add_subdirectory(firmware)
project("PowerCard")

add_custom_target("flash" COMMAND ${CMAKE_SOURCE_DIR}/script/flash_all.sh
						  DEPENDS bootloader/bootloader.bin firmware/firmware.bin)

add_custom_target("upgrade" COMMAND ${CMAKE_SOURCE_DIR}/script/iap_upgrade.sh
							DEPENDS firmware/firmware.bin)

