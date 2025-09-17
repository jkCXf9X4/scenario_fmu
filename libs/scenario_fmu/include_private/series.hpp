
#pragma once

#include "string.hpp"

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <format>
#include <iostream>

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

    struct SeriesData
    {
        Interpolation interpolation = Interpolation::Linear;
        size_t access_index = 0; // store last access for reference
        size_t size = 0;         // store last access for reference
        std::vector<double> times;
        std::vector<double> values;
    };

    // Parse scenario input like "[0;0;0][1;4;5][2;3;3][2.01;;4][3;3;3]"
    // - First field is time
    static std::vector<SeriesData> parse_scenario(std::string input)
    {
        if (input.empty())
        {
            throw std::runtime_error("No scenario found, make sure to set parameters before ExitInitializationMode");
        }

        input.erase(0, 1);                  // remove '['
        input.erase(input.length() - 1, 1); // remove ']'
        auto groups = split(input, "][");

        size_t nr_variables = split(groups[0], ";").size();
        std::vector<SeriesData> out(nr_variables);

        std::vector<std::vector<std::optional<double>>> temp_rows;
        for (const auto &g : groups)
        {
            auto fields = split(g, ";");
            if (nr_variables != fields.size())
            {
                throw std::runtime_error(std::format("Field {} has the wrong number of variables: Expected {}, got {}", g, nr_variables, fields.size()));
            }
            auto time = parse_double_opt(trim(fields[0]));
            if (!time.has_value())
            {
                throw std::runtime_error("Time must be specified for all scenario values");
            }

            for (int variable = 0; variable < nr_variables; ++variable)
            {
                auto var = parse_double_opt(trim(fields[variable]));
                // std::cout << "Parse scenario (" << variable << ") ";
                if (var.has_value())
                {
                    out[variable].times.push_back(time.value());
                    out[variable].values.push_back(var.value());
                    out[variable].size += 1;
                    // std::cout << time.value() << ":" << var.value();
                }
                // std::cout << std::endl;
            }
        }
        return out;
    }

    // Parses interpolation string like "[L;L;ZOH]" where first entry (time) is ignored,
    // returning one mode per output column.
    static void parse_interpolation(std::string s, std::vector<SeriesData> &series)
    {
        if (s.empty())
        {
            // use default
            return;
        }

        s.erase(0, 1);              // remove '['
        s.erase(s.length() - 1, 1); // remove ']'
        auto fields = split(s, ";");

        if (fields.size() != series.size())
        {
            throw std::runtime_error("Variables and interpolation options must be same size");
        }

        for (size_t i = 0; i < fields.size(); ++i)
        {
            const std::string tok = trim(fields[i]);
            if (tok.empty())
            {
                series[i].interpolation = Interpolation::Linear;
            }
            else
            {
                series[i].interpolation = interpolation_from_string(tok);
            }
            // std::cout << "interpolation: " << i << " " << series[i].interpolation << std::endl;
        }
    }

    static double eval_value_at(SeriesData &sd, double time)
    {
        // empty or before first time, do nothing
        if (sd.times.empty() || time < *sd.times.begin())
        {
            return 0.0;
        }

        // interpolation territory
        for (size_t index = sd.access_index + 1; index < sd.size - 1; ++index)
        {
            auto t0 = sd.times[index];
            auto v0 = sd.values[index];
            auto t1 = sd.times[index + 1];
            auto v1 = sd.values[index + 1];

            sd.access_index = index;

            // std::cout << "Searching" << std::endl;

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
