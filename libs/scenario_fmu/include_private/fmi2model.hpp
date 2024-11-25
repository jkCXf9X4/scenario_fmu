#pragma once

#include "fmi2.h"

#include <string>

namespace FMI2
{

    // not part of the standard
    typedef enum
    {
        StartAndEnd = 1 << 0,
        Instantiated = 1 << 1,
        InitializationMode = 1 << 2,

        // ME states
        EventMode = 1 << 3,
        ContinuousTimeMode = 1 << 4,

        // CS states
        StepComplete = 1 << 5,
        StepInProgress = 1 << 6,
        StepFailed = 1 << 7,
        StepCanceled = 1 << 8,

        Terminated = 1 << 9,
    } ModelState;

    class fmi2Experiment
    {
    public:
        fmi2Experiment() {}
        ~fmi2Experiment() {}

        fmi2Boolean toleranceDefined;
        fmi2Boolean stopTimeDefined;
        fmi2Real tolerance;

        fmi2Real startTime;
        fmi2Real stopTime;
        fmi2Real time;
    };

    class fmi2Model
    {
    public:
        fmi2Model() {}
        ~fmi2Model() {}

        template <class T>
        static T *from_component(fmi2Component c)
        {
            return static_cast<T*>(c);
        }

        std::string name;
        fmi2Type type;
        std::string GUID;
        std::string resourceLocation;
        const fmi2CallbackFunctions *callbacks;
        fmi2Boolean visible;
        fmi2Boolean loggingOn;

        fmi2Status status;
        fmi2StatusKind status_kind;

        fmi2ComponentEnvironment componentEnvironment;

        ModelState state;

        fmi2Experiment *experiment;
    };
}
