from __future__ import annotations

import argparse
import csv
from pathlib import Path
import sys

from fmpy import read_model_description, simulate_fmu


"""
Run a Scenario FMU with FMPy, save results to CSV and optionally plot outputs.

CLI entry point: `scenario-fmu-run`.
"""

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--fmu", default="build/scenario.fmu", help="Path to .fmu file")
    ap.add_argument(
        "--scenario-input",
        default="",
        help="Scenario input string, e.g. ",
    )
    ap.add_argument("--start", type=float, default=0.0, help="Start time")
    ap.add_argument("--stop", type=float, default=10.0, help="Stop time")
    ap.add_argument(
        "--step", type=float, default=0.01, help="Communication step / output interval"
    )
    ap.add_argument(
        "--out-dir",
        default="reference_results",
        help="Directory to write CSV and plots",
    )
    args = ap.parse_args()

    fmu_path = Path(args.fmu)
    if not fmu_path.exists():
        print(f"error: FMU not found at {fmu_path}", file=sys.stderr)
        return 2
    else:
        print(f"FMU: {fmu_path.absolute()}")

    md = read_model_description(str(fmu_path))
    outputs = [
        v for v in md.modelVariables if getattr(v, "causality", None) == "output"
    ]
    outputs.sort(key=lambda v: getattr(v, "valueReference", 0))

    output_names = [v.name for v in outputs]
    print(f"output_names={output_names}")

    parameters = [
        v for v in md.modelVariables if getattr(v, "causality", None) == "parameter"
    ]
    print(f"{parameters=}")

    start_values = {}
    if args.scenario_input:
        start_values = {"scenario_input": args.scenario_input}
    else:
        start_values = {parameters[0].name : parameters[0].start}
        pass

    print(f"start_values={start_values}")

    res = simulate_fmu(
        filename=str(fmu_path),
        start_time=args.start,
        stop_time=args.stop,
        step_size=args.step,
        fmi_type="CoSimulation",
        output=output_names,
        start_values=start_values,
    )

    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    csv_path = out_dir / "results.csv"
    headers = ["time"] + output_names
    with csv_path.open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(headers)
        for row in res:
            writer.writerow([row["time"]] + [row[name] for name in output_names])
    print(f"Saved CSV: {csv_path}")

    try:
        import matplotlib.pyplot as plt

        t = res["time"]
        plt.figure(figsize=(10, 5))
        for name in ["time"] + output_names:
            y = res[name]
            plt.plot(t, y, label=name)
        plt.xlabel("time [s]")
        plt.ylabel("outputs")
        plt.title("Scenario FMU outputs")
        plt.legend(ncol=2, fontsize="small")
        plt.grid(True, linestyle="--", alpha=0.4)
        png_path = out_dir / "results.png"
        plt.tight_layout()
        plt.savefig(png_path, dpi=150)
        plt.close()
        print(f"Saved plot: {png_path}")
    except Exception as e:
        print(f"warning: plotting skipped ({e}). Install matplotlib to enable plots.")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
