// Public convenience header for Scenario FMU
// Exposes value reference mapping and limits for users/tests.

#pragma once

namespace scenario_fmu
{
    // Value references for parameters
    inline constexpr unsigned int kVrScenarioInput = 0;     // string: "[t;v1;v2]..."
    inline constexpr unsigned int kVrInterpolation = 1;     // string: "[;L;ZOH]" per-column

    // Outputs start at this value reference and continue sequentially.
    inline constexpr unsigned int kVrFirstOutput = 2;       // real outputs 1..kMaxOutputs

    // Upper bound for number of outputs available from the FMU.
    inline constexpr unsigned int kMaxOutputs = 1000;
}

