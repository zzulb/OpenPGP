cmake_minimum_required(VERSION 3.6.0)

add_library(PKATests OBJECT
    PKAs.cpp
    dsa.cpp
    elgamal.cpp
    rsa.cpp)

file(COPY testvectors DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
