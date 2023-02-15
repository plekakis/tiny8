# MIT License
# 
# Copyright(c) 2023, Pantelis Lekakis
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this softwareand associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
# 
# The above copyright noticeand this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
cmake_minimum_required (VERSION 3.8)

project (Sample)

set(CMAKE_CXX_STANDARD 20)

# Add source to this project's executable.
add_executable (Sample "tiny8_sample.cpp" "../include/tiny8.h")

# Support both 32 and 64 bit builds
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(SDL2_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/../dependencies/SDL/lib/x64/SDL2.lib;${CMAKE_CURRENT_LIST_DIR}/../dependencies/SDL/lib/x64/SDL2main.lib")
else ()
  set(SDL2_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/../dependencies/SDL/lib/x86/SDL2.lib;${CMAKE_CURRENT_LIST_DIR}/../dependencies/SDL/lib/x86/SDL2main.lib")
endif ()
set(SDL2_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/../dependencies/SDL/include")

set(TINY8_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/../include")

target_include_directories(Sample PUBLIC ${SDL2_INCLUDE} ${TINY8_INCLUDE})
target_link_libraries(Sample PUBLIC ${SDL2_LIBRARIES})   
