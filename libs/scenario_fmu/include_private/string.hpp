
#pragma once

#include <string>
#include <optional>
#include <vector>

namespace
{
    static std::string trim(const std::string &s)
    {
        auto b = s.begin();
        while (b != s.end() && std::isspace(static_cast<unsigned char>(*b)))
        {
            ++b;
        }
        auto e = s.end();
        do
        {
            if (e == b)
                break;
            --e;
        } while (e != b && std::isspace(static_cast<unsigned char>(*e)));
        if (b == s.end())
        {
            return std::string();
        }
        return std::string(b, e + 1);
    }

    static std::optional<double> parse_double_opt(const std::string &s)
    {
        if (s.empty())
            return std::nullopt;
        char *end = nullptr;
        const double v = std::strtod(s.c_str(), &end);
        if (end == s.c_str())
            return std::nullopt; // no parse
        return v;
    }

    static std::vector<std::string> split(std::string s, const std::string &delimiter)
    {
        std::vector<std::string> tokens;
        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos)
        {
            token = s.substr(0, pos);
            tokens.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        tokens.push_back(s);

        return tokens;
    }
}