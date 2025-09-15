
#pragma once

#include <vector>

namespace
{

    struct SeriesData
    {
        // times shared across all series
        std::vector<double> times;
        // values per series, size = num_series; every vector matches times.size()
        std::vector<std::vector<double>> values;
    };
}