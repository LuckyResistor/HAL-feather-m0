# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.14)

# Set the project name
project(HAL-feather-m0)

# Make sure we use the C++17 compiler standard
set(CMAKE_CXX_STANDARD 17)

# Create a static library.
add_library(HAL-feather-m0 GPIO_SAMD21.cpp GPIO_SAMD21.hpp InterruptLock_SAMD21.cpp Timer_SAMD21.cpp
        WireMaster_FeatherM0.hpp WireMaster_SAMD21.cpp WireMaster_SAMD21.hpp Watchdog_SAMD21.cpp GPIO_Pin_SAMD21.hpp
        GPIO_Pin_FeatherM0.hpp FreeMemory_SAMD21.cpp ExtInt_SAMD21.hpp ExtInt_SAMD21.cpp ClockCycles.hpp
        Reset_SAMD21.cpp Reset_SAMD21.hpp)
add_dependencies(HAL-feather-m0 HAL-common)

add_library(HAL-feather-m0-usb-cdc SerialLine_USB.hpp SerialLine_USB.cpp)
add_dependencies(HAL-feather-m0-usb-cdc HAL-feather-m0)
add_subdirectory(usb)

function(hal_feature_usb_cdc TARGET)
    target_link_libraries(${TARGET} HAL-feather-m0-usb-cdc)
endfunction()
