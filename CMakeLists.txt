cmake_minimum_required(VERSION 3.13)
project(cxadc_source)

file(GLOB SRC "src/*.cpp")

include(${SDRPP_MODULE_CMAKE})
