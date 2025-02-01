cmake_minimum_required(VERSION 3.30)

add_subdirectory(boost)
add_library(boost_dep INTERFACE)
target_link_libraries(boost_dep INTERFACE Boost::regex Boost::container)
