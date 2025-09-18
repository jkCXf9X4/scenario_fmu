
#pragma once

#include "string.hpp"

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <format>
#include <iostream>
#include <sstream>
#include <locale>

namespace
{
    enum Interpolation
    {
        Zoh,
        Linear,
        NearestNeighbor,
        Cubic
    };

    static Interpolation interpolation_from_string(const std::string &tok)
    {
        if (tok == "L")
            return Interpolation::Linear;
        if (tok == "ZOH")
            return Interpolation::Zoh;
        if (tok == "NN")
            return Interpolation::NearestNeighbor;
        if (tok == "C")
            return Interpolation::Cubic;
        // default
        return Interpolation::Linear;
    }

    static std::string interpolation_to_string(Interpolation i)
    {
        switch (i)
        {
        case Interpolation::Linear:
            return "L";
        case Interpolation::Zoh:
            return "ZOH";
        case Interpolation::NearestNeighbor:
            return "NN";
        case Interpolation::Cubic:
            return "C";
        default:
            return "L";
        }
    }

    struct SeriesData
    {
        Interpolation interpolation = Interpolation::Linear;
        size_t access_index = 0; // store last access for reference
        size_t size = 0;         // store last access for reference
        std::string name;
        std::vector<double> times;
        std::vector<double> values;

        // Convert a SeriesData back into the serialized line format:
        // name; <InterpToken>; t0,v0; t1,v1; ...
        std::string to_string()
        {
            std::ostringstream oss;
            // Use classic locale to enforce '.' as decimal separator
            oss.imbue(std::locale::classic());

            oss << name << "; " << interpolation_to_string(interpolation);
            const size_t n = size;
            for (size_t i = 0; i < n && i < times.size() && i < values.size(); ++i)
            {
                oss << "; " << times[i] << "," << values[i];
            }
            return oss.str();
        }
    };

    // Parse scenario input
    static std::vector<SeriesData> parse_scenario(std::string input)
    {
        if (input.empty())
        {
            throw std::runtime_error("No scenario found, make sure to set parameters before ExitInitializationMode");
        }

        auto groups = split(input, "\n");

        size_t nr_variables = groups.size();
        std::vector<SeriesData> out;

        std::vector<std::vector<double>> temp_rows;
        for (const auto &g : groups)
        {
            auto fields = split(g, ";");

            SeriesData d;
            d.name = trim(fields[0]);
            d.interpolation = interpolation_from_string(trim(fields[1]));

            for (int field_index = 2; field_index < fields.size(); field_index++)
            {
                auto coordinates = split(fields[field_index], ",");
                auto x = parse_double_opt(trim(coordinates[0])).value();
                auto y = parse_double_opt(trim(coordinates[1])).value();
                d.times.push_back(x);
                d.values.push_back(y);
                d.size += 1;
            }
            // std::cout << d.to_string() << std::endl;
            out.push_back(std::move(d));
        }

        return out;
    }

    static int counter = 0;

    static double eval_value_at(SeriesData &sd, double time)
    {
        // empty or before first time, do nothing
        if (sd.times.empty() || time < *sd.times.begin())
        {
            return 0.0;
        }

        // interpolation territory
        for (size_t index = sd.access_index; index < sd.size - 1; ++index)
        {
            auto t0 = sd.times[index];
            auto v0 = sd.values[index];

            auto t1 = sd.times[index + 1];
            auto v1 = sd.values[index + 1];

            // std::cout << "Searching" << counter++ << std::endl;

            sd.access_index = index;

            if (t0 == time)
            {
                // std::cout << "OnPoint: Returning value " << v0 << " at " << time << std::endl;
                return v0;
            }
            else if (t0 < time && t1 > time)
            {
                if (sd.interpolation == Interpolation::Zoh)
                {
                    // std::cout << "ZOH: Returning value " << v0 << " at " << time << std::endl;
                    return v0;
                }

                if (sd.interpolation == Interpolation::NearestNeighbor)
                {
                    auto value = (std::abs(time - t0) <= std::abs(time - t1)) ? v0 : v1;
                    // std::cout << "NN: Returning value " << value << " at " << time << std::endl;
                    return value;
                }
                if (sd.interpolation == Interpolation::Linear)
                {
                    const double dt = (t1 - t0);
                    if (dt == 0.0)
                    {
                        return v1;
                    }
                    const double alpha = (time - t0) / dt;
                    auto value = v0 + alpha * (v1 - v0);
                    // std::cout << "Linear: Returning value " << value << " at " << time << std::endl;
                    return value;
                }
            }
        }

        // std::cout << "Extrapolate after last point, use zero order hold for all, time " << time  << std::endl;
        return sd.values[sd.access_index + 1];
    }
}
