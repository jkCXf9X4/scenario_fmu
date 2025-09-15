#!/usr/bin/env python3
"""
Run a Scenario FMU with FMPy, save results to CSV and plot outputs.

Requirements:
  - pip install fmpy matplotlib

Example:
  python3 scripts/run_fmu.py \
    --fmu build/scenario.fmu \
    --scenario-input "[0;0;0][1;4;5][2;3;3][2.01;;4][3;3;3]" \
    --interpolation "[;L;ZOH]" \
    --start 0 --stop 3 --step 0.01 \
    --out-dir build/run
"""

import argparse
import csv
import os
from pathlib import Path
import sys
from fmpy import read_model_description, simulate_fmu


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--fmu", default="build/scenario.fmu", help="Path to .fmu file")
    ap.add_argument("--scenario-input", default="[0;0;0][1;4;5][2;3;3][2.01;;4][3;3;3]", help="Scenario input string, e.g. '[0;0;0][1;4;5]' ...")
    ap.add_argument("--interpolation", default="[;L;ZOH]", help="Interpolation string, e.g. '[;L;ZOH]' (default ZOH)")
    ap.add_argument("--start", type=float, default=0.0, help="Start time")
    ap.add_argument("--stop", type=float, default=10.0, help="Stop time")
    ap.add_argument("--step", type=float, default=0.01, help="Communication step / output interval")
    ap.add_argument("--out-dir", default="run_output", help="Directory to write CSV and plots")
    args = ap.parse_args()

    fmu_path = Path(args.fmu)
    if not fmu_path.exists():
        print(f"error: FMU not found at {fmu_path}", file=sys.stderr)
        return 2
    else:
        print(f"FMU: {fmu_path.absolute()}")

    # Read modelDescription to discover outputs
    md = read_model_description(str(fmu_path))
    outputs = [v for v in md.modelVariables if getattr(v, 'causality', None) == 'output']

    # Sort by valueReference to align y1..yN
    # only use the number of outputs that are parameterized
    outputs.sort(key=lambda v: getattr(v, 'valueReference', 0))
    parameterized_variables = len(args.scenario_input.removeprefix("[").removesuffix("]") .split("][")[0].split(";"))-1

    output_names = [v.name for v in outputs[:parameterized_variables]]

    # Start values for string parameters
    start_values = {
        'scenario_input': args.scenario_input,
        'interpolation': args.interpolation,
    }
    print(f"start_values: {start_values}")

    # Run co-simulation
    res = simulate_fmu(
        filename=str(fmu_path),
        start_time=args.start,
        stop_time=args.stop,
        step_size=args.step,
        fmi_type='CoSimulation',
        output=output_names,  # record only outputs
        start_values=start_values,
    )

    # Prepare output dir
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    # Save CSV
    csv_path = out_dir / 'results.csv'
    headers = ['time'] + output_names
    with csv_path.open('w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(headers)
        # res is a numpy structured array with fields ['time'] + output_names
        for row in res:
            writer.writerow([row['time']] + [row[name] for name in output_names])
    print(f"Saved CSV: {csv_path}")

    # Plot
    try:
        import matplotlib.pyplot as plt
        import numpy as np
        t = res['time']
        plt.figure(figsize=(10, 5))
        for name in output_names:
            y = res[name]
            plt.plot(t, y, label=name)
        plt.xlabel('time [s]')
        plt.ylabel('outputs')
        plt.title('Scenario FMU outputs')
        plt.legend(ncol=2, fontsize='small')
        plt.grid(True, linestyle='--', alpha=0.4)
        png_path = out_dir / 'results.png'
        plt.tight_layout()
        plt.savefig(png_path, dpi=150)
        plt.close()
        print(f"Saved plot: {png_path}")
    except Exception as e:
        print(f"warning: plotting skipped ({e}). Install matplotlib to enable plots.")

    return 0


if __name__ == '__main__':
    raise SystemExit(main())

