cmake_minimum_required(VERSION 3.16)
project(fluffyOS)

set(CMAKE_CXX_STANDARD 14)

add_executable(fluffyOS
        defs.h
        global.cpp
        os.cpp
        resource.cpp
        rtos_api.h
        sys.h
        task.cpp
        test.cpp
        event.cpp)