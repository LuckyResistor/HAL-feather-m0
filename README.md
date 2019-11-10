# Adafruit Feather M0 Hardware Access Layer

This is an universal hardware access layer for the Adafruit Feather M0 platform.

The idea of this HAL is to replace the Arduino environment with an abstraction layer, closer to the used hardware. Therefore the hardware can be used in a more efficient and direct way, but provide a comfortable set of C++ functions and classes to work with the platform. 

## How to use this HAL

To use this HAL, you need the `HAL-toolchain`. Follow the instructions in this repository to setup the toolchain correctly. Next, include this repository as a submodule in your project. The submodule should be the subdirectory `src/hal-feather-m0` in your project.

```
YourProject/
    CMakeLists.txt
    hal-toolchain/
	src/
        CoreFunctions.cpp
		Application.hpp
		Application.cpp
		hal-common/      <-- submodule
		hal-feather-m0/  <-- submodule
			[files]
```

You also need the `hal-common` subdirectory, which contains hardware independent interfaces and classes.

```
cd YourProject/
git submodule add git@github.com:LuckyResistor/HAL-common.git src/hal-common
git submodule add git@github.com:LuckyResistor/HAL-feather-m0.git src/hal-feather-m0
```

Assuming, you use the namespace `yp` for your application. The file `Application.hpp` will look like this:

```cpp
#pragma once
// [Copyright...]
namespace yp {
namespace Application {
void setup();
__attribute__((noreturn))
void loop();
}
}
```

Whitespace and API comments were removed to keep things compact. And the `Application.cpp` will look like this:

```cpp
#include "Application.hpp"
#include "hal-common/Timer.hpp"
using lr::Timer;
namespace yp {
namespace Application {
void setup()
{
    Timer::delay(100_ms);
}
void loop()
{
    while (true) {
        Timer::delay(100_ms);
    }
}
}
}
```

The file `CoreFunctions.cpp` contains the link to your `Application` namespace and is just a simple wrapper around the `setup()` and `loop()` functions.

```cpp
#include "Application.hpp"
include "hal-common/CoreFunctions.h"
void setup() {
    lr::Application::setup();
}
void loop() {
    lr::Application::loop();
}
```

## Status
This library is a work in progress. It is published merely as an inspiration and in the hope it may be useful. 

## License
Copyright (c)2019 by Lucky Resistor.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
