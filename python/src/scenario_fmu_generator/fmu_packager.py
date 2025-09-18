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


class ScenarioFmuPackager:
    def __init__(self, model_id: str, model_name: str, guid: str):
        self.model_id = model_id
        self.model_name = model_name
        self.guid = guid or str(uuid.uuid4())
        self.version = __version__

        # Always add local time as first output
        self.time_variables = [Variable("t", "L", [[0.0, 0.0], [1000000, 1000000]])]
        # Default
        self.use_default = True
        self.variables = [
            Variable(f"y{i + 1}", "ZOH", [[0.0, 0.0]]) for i in range(1000)
        ]

    def add_raw(self, scenario_data: str):
        if self.use_default:
            self.variables = []
            self.use_default = False
    
        self.variables += Variables.from_string(scenario_data)

    def add_variable(self, variable: Variable):
        if self.use_default:
            self.variables = []
            self.use_default = False

        self.variables += variable

    def build(self, output: str):
        output = Path(output)

        print("Locate shared library")
        lib_src = packaged_library_path(self.model_id)
        if lib_src.exists():
            print(f"- Using packaged library: {lib_src}")
        else:
            print(f"error: shared library not found. Tried {lib_src}", file=sys.stderr)
            return 2

        variables = self.time_variables + self.variables

        md = generate_model_description(
            self.model_name, self.model_id, self.guid, variables, self.version
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
            output.parent.mkdir(parents=True, exist_ok=True)
            with ZipFile(output, "w", compression=ZIP_DEFLATED) as zf:
                print("-- Add modelDescription.xml")
                zf.write(tmp / "modelDescription.xml", arcname="modelDescription.xml")
                print("-- Add binaries")
                for root, _dirs, files in os.walk(tmp / "binaries"):
                    for f in files:
                        p = Path(root) / f
                        arc = p.relative_to(tmp)
                        zf.write(p, arcname=str(arc))

        print(f"Created FMU: {output}")
        print(f"  modelIdentifier: {self.model_id}")
        print(f"  modelName:      {self.model_name}")
        print(f"  guid:           {self.guid}")
        print(f"  outputs:        {len(variables)}")
        if output.exists():
            print("Successfully created")
        return True
