
#pragma once

#include <string>
#include <optional>
#include <vector>

namespace
{

    static std::string trim(const std::string &s)
    {
        auto b = s.begin();
        while (b != s.end() && std::isspace(static_cast<unsigned char>(*b))) ++b;
        auto e = s.end();
        do {
            if (e == b) break;
            --e;
        } while (e != b && std::isspace(static_cast<unsigned char>(*e)));
        if (b == s.end()) return std::string();
        return std::string(b, e + 1);
    }

    static bool iequals(const std::string &a, const std::string &b)
    {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
        {
            if (std::tolower(static_cast<unsigned char>(a[i])) !=
                std::tolower(static_cast<unsigned char>(b[i])))
                return false;
        }
        return true;
    }

    static std::optional<double> parse_double_opt(const std::string &s)
    {
        if (s.empty()) return std::nullopt;
        char *end = nullptr;
        const double v = std::strtod(s.c_str(), &end);
        if (end == s.c_str()) return std::nullopt; // no parse
        return v;
    }

    static std::vector<std::string> split(const std::string &s, char delim)
    {
        std::vector<std::string> parts;
        std::string cur;
        for (char ch : s)
        {
            if (ch == delim)
            {
                parts.emplace_back(cur);
                cur.clear();
            }
            else
            {
                cur.push_back(ch);
            }
        }
        parts.emplace_back(cur);
        return parts;
    }
}