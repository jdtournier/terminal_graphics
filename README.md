# Terminal Graphics

A simple header-only function to render images to terminal using the sixel
protocol.

## Requirements

This should compile using any standard-compliant C++20 compiler, using a
command such as:

```
g++ -std=c++20 demo.cpp -o demo
```

To work, this requires a sixel-capable terminal, such as:
- on Linux: wezterm, xterm, mlterm
- on macOS: iTerm2
- on Windows: minTTY


## Usage

- copy the `terminal_graphics.h` file into your project
- include the file in the relevant code file
- add rendering calls to your code
- run in a sixel-capable terminal (see list above)

See the [demo program](demo.cpp) for example usage. This produces the output
shown in the screenshot below.


## Demonstration

![sixel render in terminal](screenshot.png)


# Documentation

[Click here for the doxygen-generated
documentation](https://jdtournier.github.io/terminal_graphics/)
