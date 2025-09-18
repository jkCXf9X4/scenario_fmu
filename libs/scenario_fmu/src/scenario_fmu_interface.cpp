
#include "fmi2.h"

// Internal helper base class capturing FMI2 model data/state
#include "fmi2model.hpp"

// Utils
#include "series.hpp"
#include "string.hpp"

#include <vector>
#include <string>
#include <cstring>
#include <optional>
#include <algorithm>
#include <cctype>
#include <format>
#include <exception>
#include <stdexcept>

namespace
{
    // Value references for parameters
    inline constexpr unsigned int vrScenarioInput = 0;

    // - Outputs start at this value reference and continue sequentially.
    // time is the first ouput
    inline constexpr unsigned int vrFirstOutput = 1;

    class Model : public FMI2::fmi2Model
    {
    public:
        Model()
            : outputs_count(0), current_time(0.0)
        {
            experiment = new FMI2::fmi2Experiment();
            experiment->toleranceDefined = fmiFalse;
            experiment->stopTimeDefined = fmiFalse;
            experiment->tolerance = 0.0;
            experiment->startTime = 0.0;
            experiment->stopTime = 0.0;
            experiment->time = 0.0;
        }

        ~Model()
        {
            delete experiment;
        }

        // Parameters
        std::string scenario_input;      // raw string

        // Parsed
        std::vector<SeriesData> series;
        unsigned int outputs_count;

        // Time state
        double current_time;
    };
}

extern "C" {

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
    model->resourceLocation = fmuResourceLocation ? std::string(fmuResourceLocation) : std::string();
    model->callbacks = functions;
    model->visible = visible;
    model->loggingOn = loggingOn;

    model->state = FMI2::Instantiated;
    return model;
}

void fmi2FreeInstance(fmi2Component comp)
{
    auto *model = Model::from_component<Model>(comp);
    delete model;
}

/* Enter and exit initialization mode, terminate and reset */
fmi2Status fmi2SetupExperiment(fmi2Component comp,
                               fmi2Boolean toleranceDefined,
                               fmi2Real tolerance,
                               fmi2Real startTime,
                               fmi2Boolean stopTimeDefined,
                               fmi2Real stopTime)
{
    auto *model = Model::from_component<Model>(comp);
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
    auto *model = Model::from_component<Model>(comp);
    model->state = FMI2::InitializationMode;
    return fmi2OK;
}

fmi2Status fmi2ExitInitializationMode(fmi2Component comp)
{
    auto *model = Model::from_component<Model>(comp);

    model->series = parse_scenario(model->scenario_input);
    model->outputs_count = static_cast<unsigned int>(model->series.size());

    model->state = FMI2::StepComplete;
    return fmi2OK;
}

fmi2Status fmi2Terminate(fmi2Component comp)
{
    auto *model = Model::from_component<Model>(comp);
    model->state = FMI2::Terminated;
    return fmi2OK;
}

/* Providing independent variables and re-initialization of caching */
fmi2Status fmi2SetTime(fmi2Component comp,
                       fmi2Real time)
{
    auto *model = Model::from_component<Model>(comp);
    model->current_time = time;
    if (model->experiment)
        model->experiment->time = time;
    return fmi2OK;
}

fmi2Status fmi2SetString(fmi2Component comp,
                         const fmi2ValueReference vr[], size_t nvr,
                         const fmi2String value[])
{
    auto *model = Model::from_component<Model>(comp);
    for (size_t i = 0; i < nvr; ++i)
    {
        const auto ref = vr[i];
        const char *val_c = value[i] ? value[i] : "";
        if (ref == vrScenarioInput)
        {
            model->scenario_input = std::string(val_c);
        }
    }
    return fmi2OK;
}

fmi2Status fmi2DoStep(fmi2Component comp,
                      fmi2Real currentCommunicationPoint,
                      fmi2Real communicationStepSize,
                      fmi2Boolean noSetFMUStatePriorToCurrentPoint)
{
    auto *model = Model::from_component<Model>(comp);
    const double new_time = currentCommunicationPoint + communicationStepSize;
    model->current_time = new_time;
    if (model->experiment)
        model->experiment->time = new_time;
    model->state = FMI2::StepComplete;
    return fmi2OK;
}

/* Getting and setting variable values */
fmi2Status fmi2GetReal(fmi2Component comp,
                       const fmi2ValueReference vr[], size_t nvr,
                       fmi2Real value[])
{
    auto *model = Model::from_component<Model>(comp);
    auto status = fmi2OK;

    for (size_t i = 0; i < nvr; ++i)
    {
        const unsigned int index = vr[i] - vrFirstOutput; // 0-based
        if (index >= 0 && index < model->outputs_count)
        {
            value[i] = eval_value_at(model->series[index], model->current_time);
        }
        else
        {
            // Not an output
            // return 0
            value[i] = 0.0;
            status = fmi2Warning;
        }
    }
    return status;
}

fmi2Status fmi2Reset(fmi2Component comp)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetDebugLogging(fmi2Component comp,
                               fmi2Boolean loggingOn,
                               size_t nCategories,
                               const fmi2String categories[])
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Getting and setting the internal FMU state */
fmi2Status fmi2GetFMUstate(fmi2Component comp,
                           fmi2FMUstate *FMUstate)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetFMUstate(fmi2Component comp,
                           fmi2FMUstate FMUstate)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2FreeFMUstate(fmi2Component comp,
                            fmi2FMUstate *FMUstate)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SerializedFMUstateSize(fmi2Component comp,
                                      fmi2FMUstate FMUstate, size_t *size)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SerializeFMUstate(fmi2Component comp,
                                 fmi2FMUstate FMUstate,
                                 fmi2Byte serializedState[], size_t size)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2DeSerializeFMUstate(fmi2Component comp,
                                   const fmi2Byte serializedState[], size_t size,
                                   fmi2FMUstate *FMUstate)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Getting partial derivatives */
fmi2Status fmi2GetDirectionalDerivative(fmi2Component comp,
                                        const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
                                        const fmi2ValueReference vKnown_ref[], size_t nKnown,
                                        const fmi2Real dvKnown[],
                                        fmi2Real dvUnknown[])
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetContinuousStates(fmi2Component comp,
                                   const fmi2Real x[], size_t nx)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetInteger(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          fmi2Integer value[])
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetBoolean(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          fmi2Boolean value[])
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetString(fmi2Component comp,
                         const fmi2ValueReference vr[], size_t nvr,
                         fmi2String value[])
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetReal(fmi2Component comp,
                       const fmi2ValueReference vr[], size_t nvr,
                       const fmi2Real value[])
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetInteger(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          const fmi2Integer value[])
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetBoolean(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          const fmi2Boolean value[])
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Enter and exit the different modes */
fmi2Status fmi2EnterEventMode(fmi2Component comp)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2NewDiscreteStates(fmi2Component comp,
                                 fmi2EventInfo *eventInfo)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2EnterContinuousTimeMode(fmi2Component comp)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2CompletedIntegratorStep(fmi2Component comp,
                                       fmi2Boolean noSetFMUStatePriorToCurrentPoint,
                                       fmi2Boolean *enterEventMode,
                                       fmi2Boolean *terminateSimulation)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Evaluation of the model equations */
fmi2Status fmi2GetDerivatives(fmi2Component comp,
                              fmi2Real derivatives[], size_t nx)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetEventIndicators(fmi2Component comp,
                                  fmi2Real eventIndicators[], size_t ni)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetContinuousStates(fmi2Component comp,
                                   fmi2Real x[], size_t nx)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component comp,
                                             fmi2Real x_nominal[], size_t nx)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Simulating the slave */
fmi2Status fmi2SetRealInputDerivatives(fmi2Component comp,
                                       const fmi2ValueReference vr[], size_t nvr,
                                       const fmi2Integer order[],
                                       const fmi2Real value[])
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetRealOutputDerivatives(fmi2Component comp,
                                        const fmi2ValueReference vr[], size_t nvr,
                                        const fmi2Integer order[],
                                        fmi2Real value[])
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2CancelStep(fmi2Component comp)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Inquire slave status */
fmi2Status fmi2GetStatus(fmi2Component comp,
                         const fmi2StatusKind s,
                         fmi2Status *value)
{
    auto *model = Model::from_component<Model>(comp);
    if (value)
    {
        *value = fmi2OK;
    }
    return fmi2OK;
}

fmi2Status fmi2GetRealStatus(fmi2Component comp,
                             const fmi2StatusKind s,
                             fmi2Real *value)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetIntegerStatus(fmi2Component comp,
                                const fmi2StatusKind s,
                                fmi2Integer *value)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetBooleanStatus(fmi2Component comp,
                                const fmi2StatusKind s,
                                fmi2Boolean *value)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetStringStatus(fmi2Component comp,
                               const fmi2StatusKind s,
                               fmi2String *value)
{
    auto *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

} // extern "C"
