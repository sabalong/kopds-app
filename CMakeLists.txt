cmake_minimum_required(VERSION 3.25)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

project(kopds_app VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Wt CONFIG REQUIRED)

add_executable(kopds-app src/main.cpp)

target_link_libraries(kopds-app
  PRIVATE
    Wt::Wt
    Wt::HTTP
    Wt::Dbo
    Wt::DboPostgres
)

target_compile_features(kopds-app PRIVATE cxx_std_17)
