cmake_minimum_required(VERSION 3.5)
project(kkk_battle VERSION 1.0.0)

find_package(eosio.cdt)

### Generate the wasm and abi
add_contract(kkkbattle kkkbattle ${CMAKE_CURRENT_SOURCE_DIR}/kkkbattle.cpp)

target_include_directories(kkkbattle.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(kkkbattle.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
