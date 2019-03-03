cmake_minimum_required(VERSION 3.5)
project(kkk_sanguo VERSION 1.0.0)

find_package(eosio.cdt)

### Generate the wasm and abi
add_contract(sanguokkk sanguokkk ${CMAKE_CURRENT_SOURCE_DIR}/sanguokkk.cpp)

target_include_directories(sanguokkk.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(sanguokkk.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
