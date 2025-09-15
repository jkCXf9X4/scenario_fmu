#include <gtest/gtest.h>

extern "C" {
#include "fmi2.h"
}

#include "scenario_fmu.hpp"

TEST(ScenarioFMU, ParseAndInterpolate)
{
    fmi2CallbackFunctions cbs{};
    auto comp = fmi2Instantiate("inst", fmi2CoSimulation, "guid", nullptr, &cbs, fmiFalse, fmiFalse);
    ASSERT_NE(nullptr, comp);

    // Provide scenario input and interpolation
    const fmi2ValueReference vr_in[2] = { scenario_fmu::kVrScenarioInput, scenario_fmu::kVrInterpolation };
    const fmi2String values[2] = { "[0;0;0][1;4;5][2;3;3][2.01;;4][3;3;3]", "[;L;ZOH]" };
    ASSERT_EQ(fmi2OK, fmi2SetString(comp, vr_in, 2, values));

    ASSERT_EQ(fmi2OK, fmi2EnterInitializationMode(comp));
    ASSERT_EQ(fmi2OK, fmi2ExitInitializationMode(comp));

    // Step to time 1.5
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 1.0, 0.5, fmiTrue));

    // Query two outputs
    const fmi2ValueReference vr_out[2] = { scenario_fmu::kVrFirstOutput + 0, scenario_fmu::kVrFirstOutput + 1 };
    fmi2Real out_vals[2] = {0.0, 0.0};
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 2, out_vals));

    // First output uses Linear between (1,4) and (2,3) => 3.5 at 1.5
    EXPECT_NEAR(3.5, out_vals[0], 1e-9);
    // Second output uses ZOH, last <= 1.5 is 5 at t=1
    EXPECT_NEAR(5.0, out_vals[1], 1e-9);

    fmi2FreeInstance(comp);
}

