cmake_minimum_required(VERSION 3.0.0)
project(arrow_service VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

find_package(Arrow REQUIRED)
find_package(ArrowFlight REQUIRED)
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)

add_executable(
    arrow_service 
    
    main.cpp
    )

add_definitions(-w)
target_link_libraries(arrow_service PRIVATE Arrow::arrow_shared)
target_link_libraries(arrow_service PRIVATE ArrowFlight::arrow_flight_shared)
target_link_libraries(arrow_service PRIVATE gRPC::grpc)
target_link_libraries(arrow_service PRIVATE protobuf::libprotobuf)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
