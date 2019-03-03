cmake_minimum_required(VERSION 3.5)
project(kkk_token VERSION 1.0.0)

find_package(eosio.cdt)

### Generate the wasm and abi
add_contract(kkktoken kkktoken ${CMAKE_CURRENT_SOURCE_DIR}/kkktoken.cpp)

target_include_directories(kkktoken.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(kkktoken.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
