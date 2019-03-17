# Adafruit Feather M0 Hardware Access Layer
This is a universal hardware access layer for the Adafruit Feather M0 platform using the Arduino IDE.

The idea of this HAL is to replace the Arduino environment with an abstraction layer, which is closer to the used hardware. This allows to make a better use of the hardware features.

## How to use this HAL
To use this HAL, include this repository as a submodule in your project. The submodule should be a subdirectory `hal` in your Arduino project directory as shown below:

```
YourProject/
	YourProject.ino
	src/
		Application.hpp
		Application.cpp
		hal/ <-- submodule
			[files]
```

Alternatively, just copy the directory in your project, if you do not wish to use GIT for updates of this library. Still, you have to copy the directory structure from above.

**The `src` subdirectory is important!** Place all your sources, except the `.ino` file in the `src` directory. All `hal` layer directories have to be placed below the `src` directory. The Arduino environment will only compile `cpp` files inside of the `src` subdirectory. If the `hal*` directories are placed outside of the `src` directory, the implementations of the are not compiled into the project.

```
cd YourProject/
git 

The file `YourProject.ino` will look like this:
```
#include "Arduino.h"
#include "src/Application.hpp"

void setup()
{
    yp::Application::setup();
}

void loop()
{
    yp::Application::loop();
}
```

Assuming, you use the namespace `yp` for your application. The file `Application.hpp` will look like this:

```
#pragma once
// [Copyright...]
#include <Arduino.h>
namespace yp {
namespace Application {
void setup();
__attribute__((noreturn))
void loop();
}
}
```

Whitespace and API comments were removed to keep things compact. And the `Application.cpp` will look like this:

```
#include "Application.hpp"
#include "hal/Timer.hpp"
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

## Status
This library is a work in progress. It is published merely as an inspiration and in the hope it may be useful. 

## License
Copyright 2019 by Lucky Resistor.

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