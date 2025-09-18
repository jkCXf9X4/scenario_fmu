
from .variable import Variable, Variables
import xml.etree.ElementTree as ET

def generate_model_description(
    model_name: str, model_id: str, guid: str, variables: list[Variable], version: str
) -> bytes:
    root = ET.Element(
        "fmiModelDescription",
        attrib={
            "fmiVersion": "2.0",
            "modelName": model_name,
            "guid": guid,
            "author": "scenario_fmu",
            "version": version,
            "generationTool": "scenario_fmu",
            "numberOfEventIndicators": "0",
        },
    )

    ET.SubElement(
        root,
        "CoSimulation",
        attrib={
            "modelIdentifier": model_id,
            "canHandleVariableCommunicationStepSize": "true",
            "canInterpolateInputs": "false",
            "needsExecutionTool": "false",
            "canBeInstantiatedOnlyOncePerProcess": "false",
            "canNotUseMemoryManagementFunctions": "false",
            "providesDirectionalDerivative": "false",
        },
    )

    ET.SubElement(root, "DefaultExperiment", attrib={"startTime": "0.0"})

    mvars = ET.SubElement(root, "ModelVariables")

    sv0 = ET.SubElement(
        mvars,
        "ScalarVariable",
        attrib={
            "name": "scenario_input",
            "valueReference": "0",
            "causality": "parameter",
            "variability": "tunable",
        },
    )
    ET.SubElement(sv0, "String", attrib={"start": Variables.to_string(variables)})

    for i, var in enumerate(variables):
        svi = ET.SubElement(
            mvars,
            "ScalarVariable",
            attrib={
                "name": f"{var.name}",
                "valueReference": str(i + 1),
                "causality": "output",
            },
        )
        ET.SubElement(svi, "Real")

    mstr = ET.SubElement(root, "ModelStructure")
    outs = ET.SubElement(mstr, "Outputs")
    for i in range(len(variables)):
        index = 2 + i  # 1-based index into ModelVariables list
        ET.SubElement(outs, "Unknown", attrib={"index": str(index)})

    tree = ET.ElementTree(root)
    ET.indent(tree, space="\t", level=0)
    return ET.tostring(root, encoding="utf-8", xml_declaration=True)
