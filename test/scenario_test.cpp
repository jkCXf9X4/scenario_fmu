#include <gtest/gtest.h>

extern "C"
{
#include "fmi2.h"
}

::testing::AssertionResult setup(fmi2Component *out)
{
    fmi2CallbackFunctions cbs{};
    auto comp = fmi2Instantiate("inst", fmi2CoSimulation, "guid", nullptr, &cbs, fmiFalse, fmiFalse);
    if (comp == nullptr)
    {
        return ::testing::AssertionFailure() << "fmi2Instantiate returned nullptr";
    }

    // Provide scenario input and interpolation
    const fmi2ValueReference vr_in[2] = {0, 1};
    const fmi2String values[2] = {"[0;0;0][1;4;5][2;3;3][2.5;;4][3;4;3]", "[L;L;ZOH]"};
    if (fmi2SetString(comp, vr_in, 2, values) != fmi2OK)
    {
        fmi2FreeInstance(comp);
        return ::testing::AssertionFailure() << "fmi2SetString failed";
    }

    if (fmi2EnterInitializationMode(comp) != fmi2OK)
    {
        fmi2FreeInstance(comp);
        return ::testing::AssertionFailure() << "fmi2EnterInitializationMode failed";
    }
    if (fmi2ExitInitializationMode(comp) != fmi2OK)
    {
        fmi2FreeInstance(comp);
        return ::testing::AssertionFailure() << "fmi2ExitInitializationMode failed";
    }

    *out = comp;
    return ::testing::AssertionSuccess();
}

TEST(ScenarioFMU, ParseOnTime)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    const fmi2ValueReference vr_out[3] = {2, 3, 4};
    fmi2Real out_vals[3] = {0.0, 0.0, 0.0};

    // Step to time 1.0
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 0.5, 0.5, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    EXPECT_NEAR(1, out_vals[0], 1e-9);
    EXPECT_NEAR(4, out_vals[1], 1e-9);
    EXPECT_NEAR(5, out_vals[2], 1e-9);

    fmi2FreeInstance(comp);
}

TEST(ScenarioFMU, ParseOnTimeMultipleSteps)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    const fmi2ValueReference vr_out[3] = {2, 3, 4};
    fmi2Real out_vals[3] = {0.0, 0.0, 0.0};

    // Step to time 1.0
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 0.1, 0.1, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 0.2, 0.1, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 0.3, 0.1, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 0.9, 0.1, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    EXPECT_NEAR(1, out_vals[0], 1e-9);
    EXPECT_NEAR(4, out_vals[1], 1e-9);
    EXPECT_NEAR(5, out_vals[2], 1e-9);

    fmi2FreeInstance(comp);
}

TEST(ScenarioFMU, ParseInterpolate)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    const fmi2ValueReference vr_out[3] = {2, 3, 4};
    fmi2Real out_vals[3] = {0.0, 0.0, 0.0};

    // Step to time 1.5
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 1.0, 0.5, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    // First is time, its linear
    EXPECT_NEAR(1.5, out_vals[0], 1e-9);
    // Second output uses Linear between (1,4) and (2,3) => 3.5 at 1.5
    EXPECT_NEAR(3.5, out_vals[1], 1e-9);
    // Third output uses ZOH, last <= 1.5 is 5 at t=1
    EXPECT_NEAR(5, out_vals[2], 1e-9);

    fmi2FreeInstance(comp);
}

TEST(ScenarioFMU, ParseExtrapolateAfter)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    const fmi2ValueReference vr_out[3] = {2, 3, 4};
    fmi2Real out_vals[3] = {0.0, 0.0, 0.0};

    // Step to time 5.5
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 5, 0.5, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    // First is time, its linear
    EXPECT_NEAR(5.5, out_vals[0], 1e-9);
    // Second output uses Linear between (1,4) and (2,3) => 3.5 at 1.5
    EXPECT_NEAR(4, out_vals[1], 1e-9);
    // Third output uses ZOH, last <= 1.5 is 5 at t=1
    EXPECT_NEAR(3, out_vals[2], 1e-9);

    fmi2FreeInstance(comp);
}

TEST(ScenarioFMU, SearchOptimization1)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    const fmi2ValueReference vr_out[3] = {2, 3, 4};
    fmi2Real out_vals[3] = {0.0, 0.0, 0.0};

    // Step to time 5.5
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 0.5, 0.5, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 2.5, 0.5, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 2.5, 0.5, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    // First is time, its linear
    EXPECT_NEAR(3, out_vals[0], 1e-9);
    // Second output uses Linear between (1,4) and (2,3) => 3.5 at 1.5
    EXPECT_NEAR(4, out_vals[1], 1e-9);
    // Third output uses ZOH, last <= 1.5 is 5 at t=1
    EXPECT_NEAR(3, out_vals[2], 1e-9);

    fmi2FreeInstance(comp);
}
