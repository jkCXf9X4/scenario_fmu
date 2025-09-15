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
    --num-outputs 1000

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
import tempfile
import uuid
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED


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


def generate_model_description(model_name: str, model_id: str, guid: str, num_outputs: int, version: str) -> str:
    # Build ModelVariables block
    lines = []
    lines.append("  <ModelVariables>")
    lines.append(
        "    <ScalarVariable name=\"scenario_input\" valueReference=\"0\" causality=\"parameter\" variability=\"tunable\">"
    )
    lines.append("      <String />")
    lines.append("    </ScalarVariable>")
    lines.append(
        "    <ScalarVariable name=\"interpolation\" valueReference=\"1\" causality=\"parameter\" variability=\"tunable\">"
    )
    lines.append("      <String />")
    lines.append("    </ScalarVariable>")

    for i in range(num_outputs):
        vr = 2 + i
        idx = i + 1
        lines.append(
            f"    <ScalarVariable name=\"y{idx}\" valueReference=\"{vr}\" causality=\"output\">"
        )
        lines.append("      <Real />")
        lines.append("    </ScalarVariable>")
    lines.append("  </ModelVariables>")

    # ModelStructure indices refer to 1-based positions in ModelVariables
    # modelVariables indices: 1:scenario_input, 2:interpolation, outputs start at 3
    lines.append("  <ModelStructure>")
    lines.append("    <Outputs>")
    for i in range(num_outputs):
        index = 3 + i
        lines.append(f"      <Unknown index=\"{index}\"/>")
    lines.append("    </Outputs>")
    lines.append("  </ModelStructure>")

    xml = f"""<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<fmiModelDescription
  fmiVersion=\"2.0\"
  modelName=\"{model_name}\"
  guid=\"{guid}\"
  author=\"scenario_fmu\"
  version=\"{version}\"
  generationTool=\"scenario_fmu\"
  numberOfEventIndicators=\"0\">\n\n  <CoSimulation
    modelIdentifier=\"{model_id}\"
    canHandleVariableCommunicationStepSize=\"true\"
    canInterpolateInputs=\"false\"
    needsExecutionTool=\"false\"
    canBeInstantiatedOnlyOncePerProcess=\"false\"
    canNotUseMemoryManagementFunctions=\"false\"
    providesDirectionalDerivative=\"false\"/>\n\n  <DefaultExperiment startTime=\"0.0\"/>\n\n{os.linesep.join(lines)}\n\n</fmiModelDescription>\n"""
    return xml




def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--build-dir", default="build", help="CMake build directory")
    ap.add_argument("--out", default="scenario.fmu", help="Output .fmu path")
    ap.add_argument("--model-id", default="scenario", help="FMI modelIdentifier (also library base name)")
    ap.add_argument("--model-name", default="ScenarioFMU", help="Human-readable modelName")
    ap.add_argument("--num-outputs", type=int, default=1000, help="Number of real outputs to expose")
    ap.add_argument("--guid", default=None, help="GUID to embed (default: random uuid4)")
    args = ap.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    build_dir = Path(args.build_dir).resolve()
    out_fmu = Path(args.out).resolve()
    model_id = args.model_id
    model_name = args.model_name
    num_outputs = max(0, int(args.num_outputs))
    guid = args.guid or str(uuid.uuid4())
    version = read_project_version(repo_root)

    print("Locate shared library")
    lib_src = default_built_library(build_dir, model_id)
    if not lib_src.exists():
        print(f"error: shared library not found at {lib_src}", file=sys.stderr)
        return 2

    platform_folder = detect_platform_folder()
    lib_target_name = lib_name_for(model_id)

    print("Generate fmu structure and content")
    tmp = build_dir / "fmu_tmp"
    tmp.mkdir(parents=True, exist_ok=True)
    print("- Write modelDescription.xml")
    md = generate_model_description(model_name, model_id, guid, num_outputs, version)
    (tmp / "modelDescription.xml").write_text(md, encoding="utf-8")

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
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

