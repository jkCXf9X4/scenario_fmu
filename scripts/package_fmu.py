#!/usr/bin/env python3
"""
Package the Scenario FMU as a valid FMI 2.0 Co-Simulation FMU (.fmu).

- Generates modelDescription.xml with (by default) 1000 outputs.
- Copies the built shared library to binaries/<platform>/.
  Platform is one of: linux64, win64, win32, darwin64.

Usage:
  python3 scripts/package_fmu.py \
    --build-dir build \
    --out build/scenario.fmu \
    --model-id scenario \
    --model-name ScenarioFMU \
    --output-names []

Notes:
  - Value references:
      0: scenario_input (String, parameter)
      1: interpolation   (String, parameter)
      2..(2+N-1): outputs (Real, output)
  - The shared library is expected at:
      Linux:   build/libs/scenario_fmu/libscenario.so
      Windows: build\\libs\\scenario_fmu\\scenario.dll
      macOS:   build/libs/scenario_fmu/libscenario.dylib
"""

import argparse
import os
import platform
import shutil
import sys
import uuid
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import xml.etree.ElementTree as ET


def detect_platform_folder() -> str:
    sysplat = sys.platform
    machine = platform.machine().lower()
    if sysplat.startswith("linux"):
        return "linux64" if machine in ("x86_64", "amd64") else "linux32"
    if sysplat == "darwin":
        return "darwin64"
    if sysplat in ("win32", "cygwin"):
        return "win32"
    if sysplat == "win64":
        return "win64"
    # Fallback
    return "linux64"


def lib_name_for(model_id: str) -> str:
    sysplat = sys.platform
    if sysplat.startswith("linux"):
        return f"lib{model_id}.so"
    if sysplat == "darwin":
        return f"lib{model_id}.dylib"
    # windows
    return f"{model_id}.dll"


def default_built_library(build_dir: Path, model_id: str) -> Path:
    # Our CMake produces
    #   Linux/macOS: build/libs/scenario_fmu/libscenario.so or .dylib
    #   Windows:     build/libs/scenario_fmu/scenario.dll
    libname = lib_name_for(model_id)
    candidate = build_dir / "libs" / "scenario_fmu" / libname
    return candidate


def read_project_version(repo_root: Path) -> str:
    vp = repo_root / "version.txt"
    try:
        return vp.read_text(encoding="utf-8").strip()
    except Exception:
        return "0.1.0"



def generate_model_description(model_name: str, model_id: str, guid: str, output_names: list[str], version: str) -> bytes:
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
    ET.SubElement(sv0, "String", attrib={"start":"[]"})

    sv1 = ET.SubElement(
        mvars,
        "ScalarVariable",
        attrib={
            "name": "interpolation",
            "valueReference": "1",
            "causality": "parameter",
            "variability": "tunable",
        },
    )
    ET.SubElement(sv1, "String", attrib={"start":"[]"})


    for i, name in enumerate(output_names):
        svi = ET.SubElement(
            mvars,
            "ScalarVariable",
            attrib={
                "name": f"{name}",
                "valueReference": str(i+2),
                "causality": "output",
            },
        )
        ET.SubElement(svi, "Real")

    mstr = ET.SubElement(root, "ModelStructure")
    outs = ET.SubElement(mstr, "Outputs")
    for i in range(len(output_names)):
        index = 3 + i  # 1-based index into ModelVariables list
        ET.SubElement(outs, "Unknown", attrib={"index": str(index)})

    tree = ET.ElementTree(root)
    ET.indent(tree, space="\t", level=0)
    # Serialize to bytes with XML declaration
    return ET.tostring(root, encoding="utf-8", xml_declaration=True)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--build-dir", default="build", help="CMake build directory")
    ap.add_argument("--out", default="scenario.fmu", help="Output .fmu path")
    ap.add_argument("--model-id", default="scenario", help="FMI modelIdentifier (also library base name)")
    ap.add_argument("--model-name", default="ScenarioFMU", help="Human-readable modelName")
    ap.add_argument("-n", "--output-names", type=str, nargs="+", default=[], help="Output variable names")
    ap.add_argument("--guid", default=None, help="GUID to embed (default: random uuid4)")
    args = ap.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    build_dir = Path(args.build_dir).resolve()
    out_fmu = build_dir / args.out
    model_id = args.model_id
    model_name = args.model_name
    if not args.output_names:
        args.output_names = [f"y{i+1}" for i in range(1000)]
    
    args.output_names = ["t"] + args.output_names # Always add the local time
    
    num_outputs =  len(args.output_names)
    guid = args.guid or str(uuid.uuid4())
    version = read_project_version(repo_root)

    # print(f"{args.output_names=}")

    print("Locate shared library")
    lib_src = default_built_library(build_dir, model_id)
    if not lib_src.exists():
        print(f"error: shared library not found at {lib_src}", file=sys.stderr)
        return 2

    platform_folder = detect_platform_folder()
    lib_target_name = lib_name_for(model_id).removeprefix("lib")


    print("Generate fmu structure and content")
    tmp = build_dir / "fmu_tmp"
    tmp.mkdir(parents=True, exist_ok=True)
    print("- Write modelDescription.xml")
    md = generate_model_description(model_name, model_id, guid, args.output_names, version)
    (tmp / "modelDescription.xml").write_bytes(md)

    print("- Place binaries")
    bin_dir = tmp / "binaries" / platform_folder
    bin_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(lib_src, bin_dir / lib_target_name)

    print("- Pack zip")
    out_fmu.parent.mkdir(parents=True, exist_ok=True)
    with ZipFile(out_fmu, "w", compression=ZIP_DEFLATED) as zf:
        print("-- Add modelDescription.xml")
        zf.write(tmp / "modelDescription.xml", arcname="modelDescription.xml")
        print("-- Add binaries")
        for root, _dirs, files in os.walk(tmp / "binaries"):
            for f in files:
                p = Path(root) / f
                arc = p.relative_to(tmp)
                zf.write(p, arcname=str(arc))

    print(f"Created FMU: {out_fmu}")
    print(f"  modelIdentifier: {model_id}")
    print(f"  modelName:      {model_name}")
    print(f"  guid:           {guid}")
    print(f"  outputs:        {num_outputs}")
    # print(f"  outputs:        {args.output_names}")
    if out_fmu.exists():
        print("Successfully created")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

