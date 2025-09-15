#include <gtest/gtest.h>

extern "C" {
#include "fmi2.h"
}

TEST(FMI2Basics, VersionIsExposed)
{
    ASSERT_STREQ("2.0", fmi2GetVersion());
}

TEST(FMI2Basics, InstantiateAndFree)
{
    fmi2CallbackFunctions cbs{};
    auto comp = fmi2Instantiate("inst", fmi2CoSimulation, "guid", nullptr, &cbs, fmiFalse, fmiFalse);
    ASSERT_NE(nullptr, comp);
    fmi2FreeInstance(comp);
}

