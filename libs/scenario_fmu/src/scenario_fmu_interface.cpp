
#include "fmi2.h"
// Internal helper base class capturing FMI2 model data/state
#include "fmi2model.hpp"
// Public VR mapping/constants
#include "scenario_fmu.hpp"

#include <vector>
#include <string>
#include <cstring>
#include <optional>
#include <algorithm>
#include <cctype>

namespace
{
    enum class Interp
    {
        Zoh,
        Linear,
        Nearest,
    };

    struct SeriesData
    {
        // times shared across all series
        std::vector<double> times;
        // values per series, size = num_series; every vector matches times.size()
        std::vector<std::vector<double>> values;
    };

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

    // Parse scenario input like "[0;0;0][1;4;5][2;3;3][2.01;;4][3;3;3]"
    // - First field is time
    // - Empty fields mean carry-forward of last known value for that column
    static SeriesData parse_scenario(const std::string &input, unsigned int max_outputs)
    {
        SeriesData out;
        if (input.empty()) return out;

        // Extract bracket groups
        std::vector<std::string> groups;
        std::string cur;
        bool in = false;
        for (char ch : input)
        {
            if (ch == '[')
            {
                in = true;
                cur.clear();
            }
            else if (ch == ']')
            {
                if (in)
                {
                    groups.emplace_back(cur);
                    cur.clear();
                }
                in = false;
            }
            else if (in)
            {
                cur.push_back(ch);
            }
        }

        // Determine max columns (excluding time) across groups
        size_t num_outputs = 0;
        std::vector<std::vector<std::optional<double>>> temp_rows;
        for (const auto &g : groups)
        {
            auto fields = split(g, ';');
            if (fields.empty()) continue;
            std::vector<std::optional<double>> row;
            row.reserve(fields.size());
            for (const auto &f : fields)
            {
                row.emplace_back(parse_double_opt(trim(f)));
            }
            if (fields.size() > 1)
            {
                num_outputs = std::max(num_outputs, fields.size() - 1);
            }
            temp_rows.emplace_back(std::move(row));
        }

        if (num_outputs == 0 || temp_rows.empty()) return out;
        num_outputs = std::min<size_t>(num_outputs, max_outputs);

        out.values.assign(num_outputs, {});

        // Carry-forward fill
        std::vector<std::optional<double>> last_vals(num_outputs, std::nullopt);
        for (const auto &row : temp_rows)
        {
            if (row.empty() || !row[0].has_value()) continue; // need time
            const double t = row[0].value();
            out.times.emplace_back(t);

            for (size_t j = 0; j < num_outputs; ++j)
            {
                std::optional<double> v = (j + 1 < row.size()) ? row[j + 1] : std::nullopt;
                if (v.has_value()) last_vals[j] = v;
                const double filled = last_vals[j].value_or(0.0);
                out.values[j].emplace_back(filled);
            }
        }

        return out;
    }

    static Interp parse_interp_token(const std::string &tok)
    {
        if (iequals(tok, "L")) return Interp::Linear;
        if (iequals(tok, "ZOH")) return Interp::Zoh;
        if (iequals(tok, "NN")) return Interp::Nearest;
        // default
        return Interp::Zoh;
    }

    // Parses interpolation string like "[;L;ZOH]" where first entry (time) is ignored,
    // returning one mode per output column.
    static std::vector<Interp> parse_interpolation(const std::string &s, size_t num_outputs)
    {
        std::vector<Interp> modes(num_outputs, Interp::Zoh);
        if (s.empty() || num_outputs == 0) return modes;

        std::string inside;
        bool in = false;
        for (char ch : s)
        {
            if (ch == '[') { in = true; continue; }
            if (ch == ']') { if (in) break; }
            if (in) inside.push_back(ch);
        }

        auto fields = split(inside.empty() ? s : inside, ';');
        // Skip first (time); apply following to outputs
        size_t out_idx = 0;
        for (size_t i = 1; i < fields.size() && out_idx < num_outputs; ++i, ++out_idx)
        {
            const std::string tok = trim(fields[i]);
            if (tok.empty()) continue; // keep default
            modes[out_idx] = parse_interp_token(tok);
        }

        return modes;
    }

    static double eval_value_at(const SeriesData &sd, const std::vector<Interp> &modes,
                                size_t series_index, double t)
    {
        if (sd.times.empty() || series_index >= sd.values.size()) return 0.0;
        const auto &times = sd.times;
        const auto &vals = sd.values[series_index];
        if (times.size() == 1) return vals[0];

        const Interp mode = (series_index < modes.size()) ? modes[series_index] : Interp::Zoh;

        // find first time greater than t
        auto it = std::upper_bound(times.begin(), times.end(), t);
        if (it == times.begin())
        {
            // before first sample
            return vals.front();
        }
        if (it == times.end())
        {
            // after last sample
            if (mode == Interp::Nearest && times.size() >= 2)
            {
                // compare last two
                const size_t n = times.size();
                const double d1 = std::abs(t - times[n - 1]);
                const double d2 = std::abs(t - times[n - 2]);
                return (d2 <= d1) ? vals[n - 2] : vals[n - 1];
            }
            return vals.back();
        }

        const size_t idx1 = static_cast<size_t>(it - times.begin());
        const size_t idx0 = idx1 - 1;
        const double t0 = times[idx0];
        const double t1 = times[idx1];
        const double v0 = vals[idx0];
        const double v1 = vals[idx1];
        if (mode == Interp::Zoh) return v0;
        if (mode == Interp::Nearest)
        {
            return (std::abs(t - t0) <= std::abs(t - t1)) ? v0 : v1;
        }
        // Linear
        const double dt = (t1 - t0);
        if (dt == 0.0) return v1;
        const double alpha = (t - t0) / dt;
        return v0 + alpha * (v1 - v0);
    }
}

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
    std::string scenario_input; // raw string
    std::string interpolation_input; // raw string

    // Parsed
    SeriesData series;
    std::vector<Interp> modes; // per-series
    unsigned int outputs_count; // number of available outputs (<= kMaxOutputs)

    // Time state
    double current_time;

    void parse_inputs()
    {
        series = parse_scenario(scenario_input, scenario_fmu::kMaxOutputs);
        outputs_count = static_cast<unsigned int>(series.values.size());
        modes = parse_interpolation(interpolation_input, outputs_count);
    }
};

    
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
    Model *model = Model::from_component<Model>(comp);
    delete model;
}

fmi2Status fmi2SetDebugLogging(fmi2Component comp,
                               fmi2Boolean loggingOn,
                               size_t nCategories,
                               const fmi2String categories[])
{
    Model *model = Model::from_component<Model>(comp);
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
    Model *model = Model::from_component<Model>(comp);
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
    Model *model = Model::from_component<Model>(comp);
    model->state = FMI2::InitializationMode;
    return fmi2OK;
}

fmi2Status fmi2ExitInitializationMode(fmi2Component comp)
{
    Model *model = Model::from_component<Model>(comp);
    // finalize parsing upon leaving init
    model->parse_inputs();
    model->state = FMI2::StepComplete;
    return fmi2OK;
}

fmi2Status fmi2Terminate(fmi2Component comp)
{
    Model *model = Model::from_component<Model>(comp);
    model->state = FMI2::Terminated;
    return fmi2OK;
}

fmi2Status fmi2Reset(fmi2Component comp)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Getting and setting the internal FMU state */
fmi2Status fmi2GetFMUstate(fmi2Component comp,
                           fmi2FMUstate *FMUstate)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetFMUstate(fmi2Component comp,
                           fmi2FMUstate FMUstate)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2FreeFMUstate(fmi2Component comp,
                            fmi2FMUstate *FMUstate)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SerializedFMUstateSize(fmi2Component comp,
                                      fmi2FMUstate FMUstate, size_t *size)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SerializeFMUstate(fmi2Component comp,
                                 fmi2FMUstate FMUstate,
                                 fmi2Byte serializedState[], size_t size)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2DeSerializeFMUstate(fmi2Component comp,
                                   const fmi2Byte serializedState[], size_t size,
                                   fmi2FMUstate *FMUstate)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Getting partial derivatives */
fmi2Status fmi2GetDirectionalDerivative(fmi2Component comp,
                                        const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
                                        const fmi2ValueReference vKnown_ref[], size_t nKnown,
                                        const fmi2Real dvKnown[],
                                        fmi2Real dvUnknown[])
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Providing independent variables and re-initialization of caching */
fmi2Status fmi2SetTime(fmi2Component comp,
                       fmi2Real time)
{
    Model *model = Model::from_component<Model>(comp);
    model->current_time = time;
    if (model->experiment) model->experiment->time = time;
    return fmi2OK;
}

fmi2Status fmi2SetContinuousStates(fmi2Component comp,
                                   const fmi2Real x[], size_t nx)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Getting and setting variable values */
fmi2Status fmi2GetReal(fmi2Component comp,
                       const fmi2ValueReference vr[], size_t nvr,
                       fmi2Real value[])
{
    Model *model = Model::from_component<Model>(comp);
    for (size_t i = 0; i < nvr; ++i)
    {
        const auto ref = vr[i];
        if (ref >= scenario_fmu::kVrFirstOutput)
        {
            const unsigned int index = ref - scenario_fmu::kVrFirstOutput; // 0-based
            if (index < model->outputs_count)
            {
                value[i] = eval_value_at(model->series, model->modes, index, model->current_time);
            }
            else
            {
                value[i] = 0.0; // out-of-range outputs return 0
            }
        }
        else
        {
            // Not a real output; return 0.0
            value[i] = 0.0;
        }
    }
    return fmi2OK;
}

fmi2Status fmi2GetInteger(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          fmi2Integer value[])
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetBoolean(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          fmi2Boolean value[])
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetString(fmi2Component comp,
                         const fmi2ValueReference vr[], size_t nvr,
                         fmi2String value[])
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetReal(fmi2Component comp,
                       const fmi2ValueReference vr[], size_t nvr,
                       const fmi2Real value[])
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetInteger(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          const fmi2Integer value[])
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetBoolean(fmi2Component comp,
                          const fmi2ValueReference vr[], size_t nvr,
                          const fmi2Boolean value[])
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2SetString(fmi2Component comp,
                         const fmi2ValueReference vr[], size_t nvr,
                         const fmi2String value[])
{
    Model *model = Model::from_component<Model>(comp);
    for (size_t i = 0; i < nvr; ++i)
    {
        const auto ref = vr[i];
        const char *val_c = value[i] ? value[i] : "";
        if (ref == scenario_fmu::kVrScenarioInput)
        {
            model->scenario_input = std::string(val_c);
        }
        else if (ref == scenario_fmu::kVrInterpolation)
        {
            model->interpolation_input = std::string(val_c);
        }
        // ignore unknown string VRs
    }
    // Re-parse when parameters change (helpful if set after init)
    model->parse_inputs();
    return fmi2OK;
}

/* Enter and exit the different modes */
fmi2Status fmi2EnterEventMode(fmi2Component comp)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2NewDiscreteStates(fmi2Component comp,
                                 fmi2EventInfo *eventInfo)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2EnterContinuousTimeMode(fmi2Component comp)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2CompletedIntegratorStep(fmi2Component comp,
                                       fmi2Boolean noSetFMUStatePriorToCurrentPoint,
                                       fmi2Boolean *enterEventMode,
                                       fmi2Boolean *terminateSimulation)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Evaluation of the model equations */
fmi2Status fmi2GetDerivatives(fmi2Component comp,
                              fmi2Real derivatives[], size_t nx)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetEventIndicators(fmi2Component comp,
                                  fmi2Real eventIndicators[], size_t ni)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetContinuousStates(fmi2Component comp,
                                   fmi2Real x[], size_t nx)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component comp,
                                             fmi2Real x_nominal[], size_t nx)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Simulating the slave */
fmi2Status fmi2SetRealInputDerivatives(fmi2Component comp,
                                       const fmi2ValueReference vr[], size_t nvr,
                                       const fmi2Integer order[],
                                       const fmi2Real value[])
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetRealOutputDerivatives(fmi2Component comp,
                                        const fmi2ValueReference vr[], size_t nvr,
                                        const fmi2Integer order[],
                                        fmi2Real value[])
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2DoStep(fmi2Component comp,
                      fmi2Real currentCommunicationPoint,
                      fmi2Real communicationStepSize,
                      fmi2Boolean noSetFMUStatePriorToCurrentPoint)
{
    Model *model = Model::from_component<Model>(comp);
    const double new_time = currentCommunicationPoint + communicationStepSize;
    model->current_time = new_time;
    if (model->experiment) model->experiment->time = new_time;
    model->state = FMI2::StepComplete;
    return fmi2OK;
}

fmi2Status fmi2CancelStep(fmi2Component comp)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

/* Inquire slave status */
fmi2Status fmi2GetStatus(fmi2Component comp,
                         const fmi2StatusKind s,
                         fmi2Status *value)
{
    Model *model = Model::from_component<Model>(comp);
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
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetIntegerStatus(fmi2Component comp,
                                const fmi2StatusKind s,
                                fmi2Integer *value)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetBooleanStatus(fmi2Component comp,
                                const fmi2StatusKind s,
                                fmi2Boolean *value)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}

fmi2Status fmi2GetStringStatus(fmi2Component comp,
                               const fmi2StatusKind s,
                               fmi2String *value)
{
    Model *model = Model::from_component<Model>(comp);
    return fmi2OK;
}
