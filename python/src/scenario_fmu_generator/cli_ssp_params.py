
import argparse

from .ssp_parameter_builder import ParameterSetBuilder


"""CLI to generate an SSP parameter set (.ssv) from an FMU.

Discovers parameter variables in the FMU's modelDescription.xml and writes a
ParameterSet with defaults (from `start` attributes) that can be overridden
from the command line.
"""


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--name", default="Default", help="Parameter set name")
    ap.add_argument("--out", default="parameters.ssv", help="Output .ssv path")
    ap.add_argument(
        "-s",
        "--scenario-data",
        type=str,
        default="",
        help="Scenario data",
    )
    args = ap.parse_args()

    b = ParameterSetBuilder(name=args.name)
    b.add_string("scenario_input", args.scenario_data)
    b.build(args.out)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

