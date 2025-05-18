cmake_minimum_required(VERSION 3.28)
target_sources(fastnx PRIVATE
    types.h
    analyzer/irs_blow.cpp

    dynarmic_callbacks.cpp
    dynarmic_callbacks.h
    dynarmic_jit.cpp
    dynarmic_jit.h
    page_table.cpp
    page_table.h
)
