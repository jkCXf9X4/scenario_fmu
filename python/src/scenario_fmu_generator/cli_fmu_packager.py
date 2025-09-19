import argparse

from .fmu_packager import ScenarioFmuPackager

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
        "--model-name", default="scenario", help="Human-readable modelName"
    )
    ap.add_argument(
        "--guid", default=None, help="GUID to embed (default: random uuid4)"
    )
    ap.add_argument(
        "-s",
        "--scenario-data",
        type=str,
        default="",
        help="Scenario data, if empty it will create a number of generic outputs that can be parameterized",
    )
    args = ap.parse_args()

    b = ScenarioFmuPackager(args.model_id, args.model_name, args.guid)
    if args.scenario_data:
        b.add_raw(args.scenario_data)

    return b.build(args.out)


if __name__ == "__main__":
    raise SystemExit(main())
