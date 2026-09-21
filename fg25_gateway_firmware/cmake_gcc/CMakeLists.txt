# Define minimal required version of CMake.
cmake_minimum_required(VERSION "3.25")

# Project definition
project(
	fg25_gateway_firmware
	VERSION 1.0
	LANGUAGES C CXX ASM
)

# Include the definition of the slc_fg25_gateway_firmware target,
# which contains the content of the SLC project
include(fg25_gateway_firmware.cmake)

add_executable(fg25_gateway_firmware
    # Add additional sources here
)

target_include_directories(fg25_gateway_firmware PUBLIC
    # Add additional include paths here
)

target_compile_definitions(fg25_gateway_firmware PUBLIC
    # Add additional macros here
)

target_compile_options(fg25_gateway_firmware PUBLIC
    # Set additional compiler flags here
)

target_link_options(fg25_gateway_firmware PUBLIC
    # Set additional linker flags here
)

# Link with the content defined in the SLC project
target_link_libraries(fg25_gateway_firmware PRIVATE
    slc
)

# Include managed project content if available
include(fg25_gateway_firmware_project.cmake OPTIONAL RESULT_VARIABLE managed_project)
if(managed_project)
    message(STATUS "Using managed project content from ${managed_project}")
endif()

# Force the gcc linker command
set_target_properties(fg25_gateway_firmware PROPERTIES LINKER_LANGUAGE C)

# Create .bin, .hex and .s37 artifacts after building the project
add_custom_command(TARGET fg25_gateway_firmware
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_SREC_CMD} "$<TARGET_FILE:fg25_gateway_firmware>" "$<TARGET_FILE_DIR:fg25_gateway_firmware>/$<TARGET_FILE_BASE_NAME:fg25_gateway_firmware>.s37"
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_IHEX_CMD} "$<TARGET_FILE:fg25_gateway_firmware>" "$<TARGET_FILE_DIR:fg25_gateway_firmware>/$<TARGET_FILE_BASE_NAME:fg25_gateway_firmware>.hex"
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_BIN_CMD}  "$<TARGET_FILE:fg25_gateway_firmware>" "$<TARGET_FILE_DIR:fg25_gateway_firmware>/$<TARGET_FILE_BASE_NAME:fg25_gateway_firmware>.bin" 
)

# Run post-build pipeline to perform additional post-processing
if(post_build_command)
add_custom_command(TARGET fg25_gateway_firmware
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
    COMMAND ${post_build_command}
)
endif()