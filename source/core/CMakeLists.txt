cmake_minimum_required(VERSION 3.28)

target_sources(fastnx PRIVATE
    application.cpp
    application.h
    assets.cpp
    assets.h
    tarball.cpp
    tarball.h
    cache.cpp
    cache.h
    games_lists.cpp
    games_lists.h
    app_setup.cpp
    app_setup.h
)