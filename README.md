# Diesel Formats
A modern C++ library written with the intent to wholly parse and losslessly store information found within
various Diesel Engine formats such as: Bundle Databases, Modern Diesel Engine asset formats (after and including PAYDAY: The Heist) and Legacy Diesel Engine asset formats (Ballistics to Lead and Gold).

## Tools
### Diesel Formats Playground
Project where I have done/am doing all of my testing of Diesel Formats. Currently focussed on testing and understanding engine packaging formats to develop the [EWS mod](https://github.com/HW12Dev/EWS).

### Diesel Visualiser
Custom visualiser extension for the Visual Studio 2022 Debugger that implements the visualisation of the un-hashed values of Idstrings.

Hashlist paths for this tool need to be hardcoded for now, anybody wishing to use this tool will need to edit the path in `DieselIdstringVisualiserService.cpp`.

### Diesel Lookup Tool
Background process that can be summoned to a window via a tray icon that allows easy Idstring lookups and lookups into Bundle Database contents. (written with Qt6)

## License
Code in this repository is either written purposefully for use in this repository, or derived solely from reverse engineering various Diesel Engine games and is licensed under the MIT License.

Functions that have been written using reverse engineered code are annotated with their original function names where possible.
Any variable names starting with an underscore (`_`) are directly from Diesel. if a variable name does not start with an underscore, it is safe to assume that it only exists in DieselFormats' generic interpretation of Diesel Engine code.

## Disclaimer
This project is not endorsed by or affiliated with Starbreeze Studios, Lion game Lion, Fatshark or Grin.