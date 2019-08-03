# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.14)

# Set the project name
project(HAL-feather-m0)

# Make sure we use the C++17 compiler standard
set(CMAKE_CXX_STANDARD 17)

# Create a static library.
add_library(HAL-feather-m0 Chip.hpp GPIO_SAMD21.cpp GPIO_SAMD21.hpp InterruptLock_Arduino.cpp
        SerialLine_ArduinoUSB.cpp SerialLine_ArduinoUSB.hpp Timer_Arduino.cpp WireMaster_FeatherM0.hpp
        WireMaster_SAMD21.cpp WireMaster_SAMD21.hpp Watchdog_SAMD21.cpp GPIO_Pin_SAMD21.hpp GPIO_Pin_FeatherM0.hpp)

