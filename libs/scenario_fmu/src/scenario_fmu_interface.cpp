
#include "fmi2.h"
#include "fmi2model.hpp"
#include "scenario_fmu.hpp"

using namespace FMI2;
using namespace Scenario;


const char *fmi2GetTypesPlatform(void)
{
    return fmi2TypesPlatform;
}

const char *fmi2GetVersion(void)
{
    return fmi2Version;
}

fmi2Component fmi2Instantiate(fmi2String instanceName,
                              fmi2Type fmuType,
                              fmi2String fmuGUID,
                              fmi2String fmuResourceLocation,
                              const fmi2CallbackFunctions *functions,
                              fmi2Boolean visible,
                              fmi2Boolean loggingOn)
{
    auto model = new Model();
    model->name = std::string(instanceName);
    model->type = fmuType;
    model->GUID = std::string(fmuGUID);
    model->resourceLocation = fmuResourceLocation ? std::string(fmuResourceLocation) : NULL;
    model->callbacks = functions;
    model->visible = visible;
    model->loggingOn = loggingOn;

    model->state = Instantiated;
    return model;
}

void fmi2FreeInstance(fmi2Component comp)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    delete model;
}

fmi2Status fmi2SetDebugLogging(fmi2Component comp,
                               fmi2Boolean loggingOn,
                               size_t nCategories,
                               const fmi2String categories[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Enter and exit initialization mode, terminate and reset */
fmi2Status fmi2SetupExperiment(fmi2Component comp,
                               fmi2Boolean toleranceDefined,
                               fmi2Real tolerance,
                               fmi2Real startTime,
                               fmi2Boolean stopTimeDefined,
                               fmi2Real stopTime)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    model->experiment = new FMI2::fmi2Experiment();
    model->experiment->toleranceDefined = toleranceDefined;
    model->experiment->tolerance = tolerance;
    model->experiment->startTime = startTime;
    model->experiment->stopTimeDefined = stopTimeDefined;
    model->experiment->stopTime = stopTime;

    return fmi2OK;
}

fmi2Status fmi2EnterInitializationMode(fmi2Component comp)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    model->state = FMI2::InitializationMode;
    return fmi2OK;
}

fmi2Status fmi2ExitInitializationMode(fmi2Component comp)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    // model->state = 
    return fmi2OK;
}

fmi2Status fmi2Terminate(fmi2Component comp)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    model->state = FMI2::Terminated;
    return fmi2OK;
}

fmi2Status fmi2Reset(fmi2Component comp)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Getting and setting the internal FMU state */
fmi2Status fmi2GetFMUstate(fmi2Component comp,
                           fmi2FMUstate *FMUstate)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetFMUstate(fmi2Component comp,
                           fmi2FMUstate FMUstate)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2FreeFMUstate(fmi2Component comp,
                            fmi2FMUstate *FMUstate)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SerializedFMUstateSize(fmi2Component comp,
                                      fmi2FMUstate FMUstate, size_t *size)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SerializeFMUstate(fmi2Component comp,
                                 fmi2FMUstate FMUstate,
                                 fmi2Byte serializedState[], size_t size)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2DeSerializeFMUstate(fmi2Component comp,
                                   const fmi2Byte serializedState[], size_t size,
                                   fmi2FMUstate *FMUstate)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Getting partial derivatives */
fmi2Status fmi2GetDirectionalDerivative(fmi2Component comp,
                                        const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
                                        const fmi2ValueReference vKnown_ref[], size_t nKnown,
                                        const fmi2Real dvKnown[],
                                        fmi2Real dvUnknown[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Providing independent variables and re-initialization of caching */
fmi2Status fmi2SetTime(fmi2Component comp,
                       fmi2Real time)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetContinuousStates(fmi2Component comp,
                                   const fmi2Real x[], size_t nx)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Getting and setting variable values */
fmi2Status fmi2GetReal(fmi2Component comp,
                       const fmi2ValueReference vr[], size_t nvr,
                       fmi2Real value[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetInteger(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          fmi2Integer value[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetBoolean(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          fmi2Boolean value[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetString(fmi2Component comp,
                         const fmi2ValueReference vr[], size_t nvr,
                         fmi2String value[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetReal(fmi2Component comp,
                       const fmi2ValueReference vr[], size_t nvr,
                       const fmi2Real value[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetInteger(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          const fmi2Integer value[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetBoolean(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          const fmi2Boolean value[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetString(fmi2Component comp,
                         const fmi2ValueReference vr[], size_t nvr,
                         const fmi2String value[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Enter and exit the different modes */
fmi2Status fmi2EnterEventMode(fmi2Component comp)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2NewDiscreteStates(fmi2Component comp,
                                 fmi2EventInfo *eventInfo)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2EnterContinuousTimeMode(fmi2Component comp)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2CompletedIntegratorStep(fmi2Component comp,
                                       fmi2Boolean noSetFMUStatePriorToCurrentPoint,
                                       fmi2Boolean *enterEventMode,
                                       fmi2Boolean *terminateSimulation)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Evaluation of the model equations */
fmi2Status fmi2GetDerivatives(fmi2Component comp,
                              fmi2Real derivatives[], size_t nx)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetEventIndicators(fmi2Component comp,
                                  fmi2Real eventIndicators[], size_t ni)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetContinuousStates(fmi2Component comp,
                                   fmi2Real x[], size_t nx)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component comp,
                                             fmi2Real x_nominal[], size_t nx)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Simulating the slave */
fmi2Status fmi2SetRealInputDerivatives(fmi2Component comp,
                                       const fmi2ValueReference vr[], size_t nvr,
                                       const fmi2Integer order[],
                                       const fmi2Real value[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetRealOutputDerivatives(fmi2Component comp,
                                        const fmi2ValueReference vr[], size_t nvr,
                                        const fmi2Integer order[],
                                        fmi2Real value[])
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2DoStep(fmi2Component comp,
                      fmi2Real currentCommunicationPoint,
                      fmi2Real communicationStepSize,
                      fmi2Boolean noSetFMUStatePriorToCurrentPoint)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2CancelStep(fmi2Component comp)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Inquire slave status */
fmi2Status fmi2GetStatus(fmi2Component comp,
                         const fmi2StatusKind s,
                         fmi2Status *value)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetRealStatus(fmi2Component comp,
                             const fmi2StatusKind s,
                             fmi2Real *value)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetIntegerStatus(fmi2Component comp,
                                const fmi2StatusKind s,
                                fmi2Integer *value)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetBooleanStatus(fmi2Component comp,
                                const fmi2StatusKind s,
                                fmi2Boolean *value)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetStringStatus(fmi2Component comp,
                               const fmi2StatusKind s,
                               fmi2String *value)
{
    Scenario::Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}
