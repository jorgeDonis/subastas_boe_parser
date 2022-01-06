cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
project(subastas_boe_parser)

add_executable(main
    src/main.cpp
    src/DummyPersister.cpp
    src/AuctionPersister.cpp
    src/Parser.cpp
)

target_include_directories(main PRIVATE include)
target_link_libraries(main -Wl,-Bsymbolic-functions -Wl,-z,relro -lcurl -lcurlpp)

enable_cxx_compiler_flag_if_supported(-Wall)
enable_cxx_compiler_flag_if_supported(-pedantic)
enable_cxx_compiler_flag_if_supported(-O0)
enable_cxx_compiler_flag_if_supported(-g)

