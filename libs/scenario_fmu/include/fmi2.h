
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Define compilation platform, redefine to overwrite
#define fmi2TypesPlatform "default"
#define fmi2Version "2.0"

/* Values for fmiBoolean  */
#define fmiTrue 1
#define fmiFalse 0

    typedef void *fmi2Component;            /* Pointer to FMU instance       */
    typedef void *fmi2ComponentEnvironment; /* Pointer to FMU environment    */
    typedef void *fmi2FMUstate;             /* Pointer to internal FMU state */
    typedef unsigned int fmi2ValueReference;
    typedef double fmi2Real;
    typedef int fmi2Integer;
    typedef int fmi2Boolean;
    typedef char fmi2Char;
    typedef const fmi2Char *fmi2String;
    typedef char fmi2Byte;

    /* Type definitions */
    typedef enum
    {
        fmi2OK,
        fmi2Warning,
        fmi2Discard,
        fmi2Error,
        fmi2Fatal,
        fmi2Pending
    } fmi2Status;

    typedef enum
    {
        fmi2ModelExchange = 0,
        fmi2CoSimulation = 1
    } fmi2Type;

    typedef enum
    {
        fmi2DoStepStatus = 0,
        fmi2PendingStatus = 1,
        fmi2LastSuccessfulTime = 2,
        fmi2Terminated = 3
    } fmi2StatusKind;

    // Callbacks
    typedef struct
    {
        void (*logger)(fmi2ComponentEnvironment componentEnvironment,
                       fmi2String instanceName,
                       fmi2Status status,
                       fmi2String category,
                       fmi2String message, ...);
        void *(*allocateMemory)(size_t nobj, size_t size);
        void (*freeMemory)(void *obj);
        void (*stepFinished)(fmi2ComponentEnvironment componentEnvironment,
                             fmi2Status status);
        fmi2ComponentEnvironment componentEnvironment;
    } fmi2CallbackFunctions;

    typedef struct
    {
        fmi2Boolean newDiscreteStatesNeeded;
        fmi2Boolean terminateSimulation;
        fmi2Boolean nominalsOfContinuousStatesChanged;
        fmi2Boolean valuesOfContinuousStatesChanged;
        fmi2Boolean nextEventTimeDefined;
        fmi2Real nextEventTime;
    } fmi2EventInfo;

    // Common functions

    /***************************************************
    Common Functions
    ****************************************************/

    /* Inquire version numbers of header files and setting logging status */
    const char *fmi2GetTypesPlatform(void);

    const char *fmi2GetVersion(void);

    fmi2Component fmi2Instantiate(fmi2String instanceName,
                                  fmi2Type fmuType,
                                  fmi2String fmuGUID,
                                  fmi2String fmuResourceLocation,
                                  const fmi2CallbackFunctions *functions,
                                  fmi2Boolean visible,
                                  fmi2Boolean loggingOn);

    void fmi2FreeInstance(fmi2Component comp);

    fmi2Status fmi2SetDebugLogging(fmi2Component comp,
                                   fmi2Boolean loggingOn,
                                   size_t nCategories,
                                   const fmi2String categories[]);

    /* Enter and exit initialization mode, terminate and reset */
    fmi2Status fmi2SetupExperiment(fmi2Component comp,
                                   fmi2Boolean toleranceDefined,
                                   fmi2Real tolerance,
                                   fmi2Real startTime,
                                   fmi2Boolean stopTimeDefined,
                                   fmi2Real stopTime);

    fmi2Status fmi2EnterInitializationMode(fmi2Component comp);

    fmi2Status fmi2ExitInitializationMode(fmi2Component comp);

    fmi2Status fmi2Terminate(fmi2Component comp);

    fmi2Status fmi2Reset(fmi2Component comp);

    /* Getting and setting the internal FMU state */
    fmi2Status fmi2GetFMUstate(fmi2Component comp,
                               fmi2FMUstate *FMUstate);

    fmi2Status fmi2SetFMUstate(fmi2Component comp,
                               fmi2FMUstate FMUstate);

    fmi2Status fmi2FreeFMUstate(fmi2Component comp,
                                fmi2FMUstate *FMUstate);

    fmi2Status fmi2SerializedFMUstateSize(fmi2Component comp,
                                          fmi2FMUstate FMUstate, size_t *size);

    fmi2Status fmi2SerializeFMUstate(fmi2Component comp,
                                     fmi2FMUstate FMUstate,
                                     fmi2Byte serializedState[], size_t size);

    fmi2Status fmi2DeSerializeFMUstate(fmi2Component comp,
                                       const fmi2Byte serializedState[], size_t size,
                                       fmi2FMUstate *FMUstate);

    /* Getting partial derivatives */
    fmi2Status fmi2GetDirectionalDerivative(fmi2Component comp,
                                            const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
                                            const fmi2ValueReference vKnown_ref[], size_t nKnown,
                                            const fmi2Real dvKnown[],
                                            fmi2Real dvUnknown[]);

    /* Providing independent variables and re-initialization of caching */
    fmi2Status fmi2SetTime(fmi2Component comp,
                           fmi2Real time);

    fmi2Status fmi2SetContinuousStates(fmi2Component comp,
                                       const fmi2Real x[], size_t nx);

    /* Getting and setting variable values */
    fmi2Status fmi2GetReal(fmi2Component comp,
                           const fmi2ValueReference vr[], size_t nvr,
                           fmi2Real value[]);

    fmi2Status fmi2GetInteger(fmi2Component comp,
                              const fmi2ValueReference vr[], size_t nvr,
                              fmi2Integer value[]);

    fmi2Status fmi2GetBoolean(fmi2Component comp,
                              const fmi2ValueReference vr[], size_t nvr,
                              fmi2Boolean value[]);

    fmi2Status fmi2GetString(fmi2Component comp,
                             const fmi2ValueReference vr[], size_t nvr,
                             fmi2String value[]);

    fmi2Status fmi2SetReal(fmi2Component comp,
                           const fmi2ValueReference vr[], size_t nvr,
                           const fmi2Real value[]);

    fmi2Status fmi2SetInteger(fmi2Component comp,
                              const fmi2ValueReference vr[], size_t nvr,
                              const fmi2Integer value[]);

    fmi2Status fmi2SetBoolean(fmi2Component comp,
                              const fmi2ValueReference vr[], size_t nvr,
                              const fmi2Boolean value[]);

    fmi2Status fmi2SetString(fmi2Component comp,
                             const fmi2ValueReference vr[], size_t nvr,
                             const fmi2String value[]);

    /* Enter and exit the different modes */
    fmi2Status fmi2EnterEventMode(fmi2Component comp);

    fmi2Status fmi2NewDiscreteStates(fmi2Component comp,
                                     fmi2EventInfo *eventInfo);

    fmi2Status fmi2EnterContinuousTimeMode(fmi2Component comp);

    fmi2Status fmi2CompletedIntegratorStep(fmi2Component comp,
                                           fmi2Boolean noSetFMUStatePriorToCurrentPoint,
                                           fmi2Boolean *enterEventMode,
                                           fmi2Boolean *terminateSimulation);

    /* Evaluation of the model equations */
    fmi2Status fmi2GetDerivatives(fmi2Component comp,
                                  fmi2Real derivatives[], size_t nx);

    fmi2Status fmi2GetEventIndicators(fmi2Component comp,
                                      fmi2Real eventIndicators[], size_t ni);

    fmi2Status fmi2GetContinuousStates(fmi2Component comp,
                                       fmi2Real x[], size_t nx);

    fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component comp,
                                                 fmi2Real x_nominal[], size_t nx);

    /* Simulating the slave */
    fmi2Status fmi2SetRealInputDerivatives(fmi2Component comp,
                                           const fmi2ValueReference vr[], size_t nvr,
                                           const fmi2Integer order[],
                                           const fmi2Real value[]);

    fmi2Status fmi2GetRealOutputDerivatives(fmi2Component comp,
                                            const fmi2ValueReference vr[], size_t nvr,
                                            const fmi2Integer order[],
                                            fmi2Real value[]);

    fmi2Status fmi2DoStep(fmi2Component comp,
                          fmi2Real currentCommunicationPoint,
                          fmi2Real communicationStepSize,
                          fmi2Boolean noSetFMUStatePriorToCurrentPoint);

    fmi2Status fmi2CancelStep(fmi2Component comp);

    /* Inquire slave status */
    fmi2Status fmi2GetStatus(fmi2Component comp,
                             const fmi2StatusKind s,
                             fmi2Status *value);

    fmi2Status fmi2GetRealStatus(fmi2Component comp,
                                 const fmi2StatusKind s,
                                 fmi2Real *value);

    fmi2Status fmi2GetIntegerStatus(fmi2Component comp,
                                    const fmi2StatusKind s,
                                    fmi2Integer *value);

    fmi2Status fmi2GetBooleanStatus(fmi2Component comp,
                                    const fmi2StatusKind s,
                                    fmi2Boolean *value);

    fmi2Status fmi2GetStringStatus(fmi2Component comp,
                                   const fmi2StatusKind s,
                                   fmi2String *value);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif