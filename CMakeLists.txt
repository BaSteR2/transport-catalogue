cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

set(Protobuf_PREFIX_PATH
    "~/protob/Protobuf/include"            
    "~/protob/Protobuf/lib"             
    "~/protob/Protobuf/bin" )
    
list(APPEND CMAKE_PREFIX_PATH "${Protobuf_PREFIX_PATH}")

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/transport_catalogue.proto src/map_renderer.proto src/transport_router.proto)


set(CATALOGUE_FILES src/domain.h src/geo.h src/geo.cpp src/graph.h src/json.h src/json.cpp src/json_builder.h src/json_builder.cpp src/json_reader.h src/json_reader.cpp src/main.cpp src/map_renderer.h 
src/map_renderer.cpp src/ranges.h src/router.h src/svg.h src/svg.cpp src/transport_catalogue.h src/transport_catalogue.cpp src/transport_router.h 
src/transport_router.cpp src/serialization.h src/serialization.cpp)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${CATALOGUE_FILES})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})


string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
