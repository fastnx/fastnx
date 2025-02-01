#pragma once

#include <boost/container/small_vector.hpp>

#include <common/types.h>
namespace FastNx::Device {

    boost::container::small_vector<U32, 4> GetCpuValues();

    std::pair<std::string, I32> IsArchSuitable();
}