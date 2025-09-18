import argparse
import os
import shutil
import sys
import tempfile
import uuid
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED


from . import __version__
from .model_description import generate_model_description
from .utils import detect_platform_folder, lib_name_for, packaged_library_path
from .variable import Variable, Variables


class ScenarioFmuBuilder:
    def __init__(
        self, output: str, model_id: str, model_name: str, guid: str, version: str
    ):
        self.output = Path(output)
        self.model_id = model_id
        self.model_name = model_name
        self.guid = guid or str(uuid.uuid4())
        self.version = version

        # Always add local time as first output
        self.variables = [Variable("t", "L", [[0.0, 0.0], [1000000, 1000000]])]


    def add_raw(self, scenario_data: str):

        if scenario_data:
            self.variables = Variables.from_string(scenario_data)
        else:
            # Only the name is used
            self.variables += [Variable(f"y{i + 1}", "ZOH", [[0.0, 0.0]]) for i in range(1000)]

    def add_variable(self, variable: Variable):
        self.variables += variable


    def build(self):
        print("Locate shared library")
        lib_src = packaged_library_path(self.model_id)
        if lib_src.exists():
            print(f"- Using packaged library: {lib_src}")
        else:
            print(f"error: shared library not found. Tried {lib_src}", file=sys.stderr)
            return 2

        md = generate_model_description(
            self.model_name, self.model_id, self.guid, self.variables, self.version
        )

        print("Generate fmu structure and content")
        with tempfile.TemporaryDirectory(prefix="fmu_tmp") as tmp:
            print("- Write modelDescription.xml")
            tmp = Path(tmp)
            (tmp / "modelDescription.xml").write_bytes(md)

            print("- Place binaries")
            platform_folder = detect_platform_folder()
            bin_dir = tmp / "binaries" / platform_folder
            bin_dir.mkdir(parents=True, exist_ok=True)
            lib_target_name = lib_name_for(self.model_id).removeprefix("lib")
            shutil.copy2(lib_src, bin_dir / lib_target_name)

            print("- Pack zip")
            self.output.parent.mkdir(parents=True, exist_ok=True)
            with ZipFile(self.output, "w", compression=ZIP_DEFLATED) as zf:
                print("-- Add modelDescription.xml")
                zf.write(tmp / "modelDescription.xml", arcname="modelDescription.xml")
                print("-- Add binaries")
                for root, _dirs, files in os.walk(tmp / "binaries"):
                    for f in files:
                        p = Path(root) / f
                        arc = p.relative_to(tmp)
                        zf.write(p, arcname=str(arc))

        print(f"Created FMU: {self.output}")
        print(f"  modelIdentifier: {self.model_id}")
        print(f"  modelName:      {self.model_name}")
        print(f"  guid:           {self.guid}")
        print(f"  outputs:        {len(self.variables)}")
        if self.output.exists():
            print("Successfully created")
        return True


"""
Package the Scenario FMU as a valid FMI 2.0 Co-Simulation FMU (.fmu).

- Generates modelDescription.xml with configurable outputs.
- Copies the built shared library to binaries/<platform>/.

CLI entry point: `scenario-fmu-package`.
"""


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--out",
        default="./build/scenario.fmu",
        help="Output fmu path (relative to build dir if not absolute)",
    )
    ap.add_argument(
        "--model-id",
        default="scenario",
        help="FMI modelIdentifier (also library base name)",
    )
    ap.add_argument(
        "--model-name", default="ScenarioFMU", help="Human-readable modelName"
    )
    ap.add_argument(
        "--guid", default=None, help="GUID to embed (default: random uuid4)"
    )
    ap.add_argument("-s", "--scenario-data", type=str, default="", help="Scenario data, if empty it will create a number of generic outputs that can be parameterized")
    args = ap.parse_args()

    b = ScenarioFmuBuilder(
        args.out, args.model_id, args.model_name, args.guid, __version__
    )

    b.add_raw(args.scenario_data)

    return b.build()


if __name__ == "__main__":
    raise SystemExit(main())
