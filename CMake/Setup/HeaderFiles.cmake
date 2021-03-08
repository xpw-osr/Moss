function(target_include_hal_header_files IN_target)
    target_include_directories(${IN_target} PRIVATE ${MOSS_SOURCE_CODE_DIR}/HAL/include)
endfunction()

function(target_include_libcxx_header_files IN_target)
    target_include_directories(${IN_target} PRIVATE ${MOSS_SOURCE_CODE_DIR}/Libraries/LibCXX/include)
endfunction()

function(target_include_libstdcxx_header_files IN_target)
    # todo: interim solution, refactor this
    include_directories(${MOSS_SOURCE_CODE_DIR}/Libraries/LibGccStd/include/aarch64-unknown-elf)
    target_include_directories(${IN_target} PRIVATE ${MOSS_SOURCE_CODE_DIR}/Libraries/LibGccStd/include)
endfunction()

function(target_include_kernel_header_files IN_target)
    target_include_directories(${IN_target} PRIVATE ${MOSS_SOURCE_CODE_DIR}/Kernel/include)
endfunction()

function(target_include_arch_header_files IN_target IN_arch)
    target_include_directories(${IN_target} PRIVATE ${MOSS_SOURCE_CODE_DIR}/Arch/${IN_arch}/include)
endfunction()

function(target_include_raspberry_pi3_header_files IN_target)
    target_include_directories(${IN_target} PRIVATE ${MOSS_SOURCE_CODE_DIR}/Board/RaspberryPi3/include)
endfunction()

function(target_include_tests_header_files IN_target)
    target_include_directories(${IN_target} PRIVATE ${MOSS_SOURCE_CODE_DIR}/Tests/include)
endfunction()
