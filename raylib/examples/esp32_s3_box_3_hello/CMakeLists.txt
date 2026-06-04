cmake_minimum_required(VERSION 3.22)

# Minimal build - only main component and its dependencies
set(COMPONENTS main)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32s3_hello)
