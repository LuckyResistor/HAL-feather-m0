# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.14)

# Make sure we use the C++17 compiler standard
set(CMAKE_CXX_STANDARD 17)

# Add more sources to this target.
target_sources(HAL-feather-m0-usb-cdc PRIVATE CDC.cpp Setup.hpp DeviceClass.cpp DeviceClass.hpp Description.hpp
        DeviceWrapper.hpp DeviceWrapper.cpp InterruptGuard.hpp EndPointHandler.hpp DoubleBufferedEndPointHandler.hpp
        DoubleBufferedEndPointHandler.cpp DeviceInfo.hpp InterruptHandler.hpp InterruptHandler.cpp CDCDescriptor.hpp
        MSCDescriptor.hpp ACMFunctionalDescriptor.hpp CMFunctionalDescriptor.hpp PluggableUsb.hpp PluggableUsb.cpp)

