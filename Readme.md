## TODO:
* Implement interrupts (whenever a timer goes off)
* Key mappings
* `8080emu.c` contains the definition of each of the 8080 instructions. There are instructions like `ADD X` where `X`
can be different values. They are essentially the same; however, they are defined repeateadly throughout the code. You
want only one definition of the function that allows you to pass it whatever `X` may be.

* `8080emu.c` uses function `set_flags` in order to set the flags whenever an instruction requires it. In previous
versions, this function didn't exist and code was repeated everywhere. There's still some of the old code floating
around. Find it and use the function!

* check `set_flags` function
