```
  ___  ___  ___   _ __  _ __   __ _ 
 / __|/ _ \/ _ \ | '_ \| '_ \ / _` |
 \__ \  __/  __/ | |_) | | | | (_| |
 |___/\___|\___| | .__/|_| |_|\__, |
                 | |           __/ |
                 |_|          |___/ 
```
A simple command line utility that extracts useful information about
chunks in one or more PNG files. It should work well on Mac, Linux, and Windows. 
The tool can be built using the Cmake build script included in this repository.
see_png **does not have any additional dependencies**, but you will **need** a C++ 23 capable compiler
since that's the standard I decided to mess with. I mainly wrote this tool to learn more
about the PNG file format, as well as about safely handling binary file formats in C++ using
bounds-checking features and whatnot. If you're looking for a reference for a PNG parser, this might
also be useful to you. Here's some example output:

![seepng_example](https://github.com/user-attachments/assets/c004f245-c49b-4682-93f5-617c19f6e45b)
