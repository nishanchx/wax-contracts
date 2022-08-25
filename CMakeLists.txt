project(wax-dapp)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( dapp main maincontract.cpp )
target_include_directories( dapp PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( dapp ${CMAKE_SOURCE_DIR}/../ricardian )