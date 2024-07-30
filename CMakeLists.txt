# This File is only for development in IDEs and is not needed for build/compile directly

cmake_minimum_required(VERSION 3.10)
project(punycode)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Append our project's cmake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Find R package
find_package(R REQUIRED)
message("R libraries: ${R_LIBRARIES}")
message("R include dirs: ${R_INCLUDE_DIR} ${Rcpp_INCLUDE_DIR}")

# Find ICU package
find_package(ICU REQUIRED COMPONENTS uc io i18n) # Specify the components you need, such as uc, io, i18n, etc.

# Add inst/include directory for libdeflate headers and link directories
include_directories(inst/include)
link_directories(inst/lib64)

# Declare our library target
add_library(punycode SHARED src/Makevars src/punycode.cpp)

# Include directories for our project
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(punycode ${R_INCLUDE_DIR} ${Rcpp_INCLUDE_DIR} ${ICU_INCLUDE_DIRS})

# Linking libraries with our target
target_link_libraries(punycode ${R_LIBRARIES} ${ICU_LIBRARIES})
