#include <gtest/gtest.h>
#include <iostream>

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
    const fmi2ValueReference vr_in[1] = {0};
    const fmi2String values[1] = {"var1; L; 1,0; 3,0.5; 5,4; 9,2\nvar2; ZOH; 2,0; 3,0.5; 5,4; 9,2\nvar3; NN; 0,0; 1,0.5; 2,4; 3,2"};

    if (fmi2SetString(comp, vr_in, 1, values) != fmi2OK)
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

    const fmi2ValueReference vr_out[3] = {1, 2, 3};
    fmi2Real out_vals[3] = {0.0, 0.0, 0.0};

    // Step to time 1.0
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 3, 0, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    EXPECT_NEAR(0.5, out_vals[0], 1e-9);
    EXPECT_NEAR(0.5, out_vals[1], 1e-9);
    EXPECT_NEAR(2, out_vals[2], 1e-9);

    fmi2FreeInstance(comp);
}

TEST(ScenarioFMU, ParseOnTimeMultipleSteps)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    const fmi2ValueReference vr_out[3] = {1, 2, 3};
    fmi2Real out_vals[3] = {0.0, 0.0, 0.0};

    // Step to time 1.0
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 0.1, 0.1, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 0.2, 0.1, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 0.3, 0.1, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 4, 0, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    EXPECT_NEAR(2.25, out_vals[0], 1e-9);
    EXPECT_NEAR(0.5, out_vals[1], 1e-9);
    EXPECT_NEAR(2, out_vals[2], 1e-9);

    fmi2FreeInstance(comp);
}

TEST(ScenarioFMU, ParseInterpolate)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    const fmi2ValueReference vr_out[3] = {1, 2, 3};
    fmi2Real out_vals[3] = {0.0, 0.0, 0.0};

    // Step to time 1.5
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 1.0, 0.5, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    // First is time, its linear
    EXPECT_NEAR(0.125, out_vals[0], 1e-9);
    // Second output uses Linear between (1,4) and (2,3) => 3.5 at 1.5
    EXPECT_NEAR(0, out_vals[1], 1e-9);
    // Third output uses ZOH, last <= 1.5 is 5 at t=1
    EXPECT_NEAR(0.5, out_vals[2], 1e-9);

    fmi2FreeInstance(comp);
}

TEST(ScenarioFMU, ParseExtrapolateAfter)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    const fmi2ValueReference vr_out[3] = {1, 2, 3};
    fmi2Real out_vals[3] = {0.0, 0.0, 0.0};

    // Step to time 5.5
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 5, 0.5, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    // // First is time, its linear
    // EXPECT_NEAR(5.5, out_vals[0], 1e-9);
    // // Second output uses Linear between (1,4) and (2,3) => 3.5 at 1.5
    // EXPECT_NEAR(4, out_vals[1], 1e-9);
    // // Third output uses ZOH, last <= 1.5 is 5 at t=1
    // EXPECT_NEAR(3, out_vals[2], 1e-9);

    fmi2FreeInstance(comp);
}

TEST(ScenarioFMU, OutputDerivativeLinearInterpolation)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 4.0, 0.0, fmiTrue));

    const fmi2Integer orders[1] = {1};
    fmi2Real derivatives[1] = {0.0};
    fmi2ValueReference vr_out[1] = {1};

    vr_out[0] = 1;

    auto status = fmi2GetRealOutputDerivatives(comp, vr_out, 1, orders, derivatives);
    EXPECT_EQ(fmi2OK, status);
    EXPECT_NEAR(1.75, derivatives[0], 1e-9);

    vr_out[0] = 2;
    status = fmi2GetRealOutputDerivatives(comp, vr_out, 1, orders, derivatives);
    EXPECT_EQ(fmi2OK, status);
    EXPECT_NEAR(0, derivatives[0], 1e-9);

    vr_out[0] = 3;
    status = fmi2GetRealOutputDerivatives(comp, vr_out, 1, orders, derivatives);
    EXPECT_EQ(fmi2OK, status);
    EXPECT_NEAR(0, derivatives[0], 1e-9);

    vr_out[0] = 4;
    status = fmi2GetRealOutputDerivatives(comp, vr_out, 1, orders, derivatives);
    EXPECT_EQ(fmi2Warning, status);

    fmi2FreeInstance(comp);
}

TEST(ScenarioFMU, OutputDerivativeUnsupportedOrder)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 4.0, 0.0, fmiTrue));

    const fmi2ValueReference vr_out[1] = {1};
    const fmi2Integer orders[1] = {2};
    fmi2Real derivatives[1] = {123.0};

    const auto status = fmi2GetRealOutputDerivatives(comp, vr_out, 1, orders, derivatives);
    EXPECT_EQ(fmi2Warning, status);
    EXPECT_DOUBLE_EQ(0.0, derivatives[0]);

    fmi2FreeInstance(comp);
}

TEST(ScenarioFMU, OutputDerivativeInvalidReference)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 4.0, 0.0, fmiTrue));

    const fmi2ValueReference vr_out[1] = {4};
    const fmi2Integer orders[1] = {1};
    fmi2Real derivatives[1] = {42.0};

    const auto status = fmi2GetRealOutputDerivatives(comp, vr_out, 1, orders, derivatives);
    EXPECT_EQ(fmi2Warning, status);
    EXPECT_DOUBLE_EQ(0.0, derivatives[0]);

    fmi2FreeInstance(comp);
}

TEST(ScenarioFMU, SearchOptimization1)
{
    fmi2Component comp = nullptr;
    ASSERT_TRUE(setup(&comp));

    const fmi2ValueReference vr_out[3] = {1, 2, 3};
    fmi2Real out_vals[3] = {0.0, 0.0, 0.0};

    // Step to time 5.5
    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 0.5, 0.5, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 2.5, 0.5, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    ASSERT_EQ(fmi2OK, fmi2DoStep(comp, 2.5, 0.5, fmiTrue));
    ASSERT_EQ(fmi2OK, fmi2GetReal(comp, vr_out, 3, out_vals));

    // // First is time, its linear
    // EXPECT_NEAR(3, out_vals[0], 1e-9);
    // // Second output uses Linear between (1,4) and (2,3) => 3.5 at 1.5
    // EXPECT_NEAR(4, out_vals[1], 1e-9);
    // // Third output uses ZOH, last <= 1.5 is 5 at t=1
    // EXPECT_NEAR(3, out_vals[2], 1e-9);

    fmi2FreeInstance(comp);
}
