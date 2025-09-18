
import platform
import sys

from pathlib import Path

def detect_platform_folder() -> str:
    sysplat = sys.platform
    machine = platform.machine().lower()
    if sysplat.startswith("linux"):
        return "linux64" if machine in ("x86_64", "amd64", "aarch64", "arm64") else "linux32"
    if sysplat == "darwin":
        return "darwin64"
    if sysplat in ("win32", "cygwin"):
        return "win64" if machine in ("x86_64", "amd64", "aarch64", "arm64") else "win32"
    if sysplat == "win64":
        return "win64"
    return "linux64"


def lib_name_for(model_id: str) -> str:
    sysplat = sys.platform
    if sysplat.startswith("linux"):
        return f"lib{model_id}.so"
    if sysplat == "darwin":
        return f"lib{model_id}.dylib"
    return f"{model_id}.dll"


def packaged_library_path(model_id: str) -> Path:
    """Path to the library shipped inside this package for current platform."""
    plat = detect_platform_folder()
    return Path(__file__).resolve().parent / "_binaries" / plat / lib_name_for(model_id)
