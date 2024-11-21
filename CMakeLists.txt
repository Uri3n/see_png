cmake_minimum_required(VERSION 3.28)
project(see_png)
set(CMAKE_CXX_STANDARD 23)

set(SEE_PNG_SOURCE_FILES
  Src/Main.cpp
  Src/InFileRef.cpp
  Src/ConManip.cpp
  Src/Context.cpp
  Src/Carrier.cpp
  Src/Chunks.cpp
  Src/HexDump.cpp
  Src/Argparse.cpp
  Include/CompileAttrs.hpp
  Include/InFileRef.hpp
  Include/Defer.hpp
  Include/FlatBuffer.hpp
  Include/Endian.hpp
  Include/Panic.hpp
  Include/ConManip.hpp
  Include/Fmt.hpp
  Include/Carrier.hpp
  Include/Chunks.hpp
  Include/HexDump.hpp
  Include/Context.hpp
  Include/Argparse.hpp
        Src/FileCycle.cpp
        Include/FileCycle.hpp
)

add_executable(see_png ${SEE_PNG_SOURCE_FILES})

if(WIN32)
  target_compile_definitions(see_png PRIVATE SEE_PNG_WIN32)
else()
  target_compile_definitions(see_png PRIVATE SEE_PNG_POSIX)
endif()

target_include_directories(see_png PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)