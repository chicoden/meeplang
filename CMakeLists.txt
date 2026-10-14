cmake_minimum_required(VERSION 4.1)

project(meeplang)
    set(INCLUDE_DIR "include")
    set(SOURCE_DIR "src")

    add_executable(meeplang
        ${SOURCE_DIR}/main.c
    )
