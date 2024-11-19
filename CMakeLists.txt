cmake_minimum_required(VERSION 3.28)
project(see_png)
set(CMAKE_CXX_STANDARD 23)

set(SEE_PNG_SOURCE_FILES
  Src/Main.cpp
  Src/FileRef.cpp
  Src/ConManip.cpp
  Include/CompileAttrs.hpp
  Include/FileRef.hpp
  Include/Defer.hpp
  Include/FlatBuffer.hpp
  Include/Endian.hpp
  Include/Panic.hpp
  Include/ConManip.hpp
  Include/Fmt.hpp
  Src/Carrier.cpp
  Src/Chunks.cpp
  Include/Carrier.hpp
  Include/Chunks.hpp
)

add_executable(see_png ${SEE_PNG_SOURCE_FILES})

if(WIN32)
  target_compile_definitions(see_png PRIVATE SEE_PNG_WIN32)
else()
  target_compile_definitions(see_png PRIVATE SEE_PNG_POSIX)
endif()

target_include_directories(see_png PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)