
from pathlib import Path
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from typing import Iterable, Optional

SSP_SSV_NS = "http://ssp-standard.org/SSP1/ParameterValues"
SSP_SSC_NS = "http://ssp-standard.org/SSP1/SystemStructureCommon"


@dataclass
class Parameter:
    name: str
    type: str  # One of: String, Real, Integer, Boolean
    value: Optional[str] = None


class ParameterSetBuilder:
    def __init__(self, name: str = "Default") -> None:
        self.name = name
        self._params: list[Parameter] = []

    def add_(self, name: str, type_: str, value: Optional[str] = None) -> "ParameterSetBuilder":
        type_norm = type_.capitalize()
        if type_norm not in {"String", "Real", "Integer", "Boolean"}:
            raise ValueError(f"Unsupported type: {type_}")
        self._params.append(Parameter(name=name, type=type_norm, value=value))
        return self

    def add_string(self, name: str, value: str = "") -> "ParameterSetBuilder":
        return self.add_(name, "String", value)

    def add_real(self, name: str, value: str = "0") -> "ParameterSetBuilder":
        return self.add_(name, "Real", str(value))

    def add_integer(self, name: str, value: str = "0") -> "ParameterSetBuilder":
        return self.add_(name, "Integer", str(value))

    def add_boolean(self, name: str, value: str = "false") -> "ParameterSetBuilder":
        return self.add_(name, "Boolean", str(value))

    def extend(self, items: Iterable[Parameter]) -> "ParameterSetBuilder":
        for p in items:
            self.add_(p.name, p.type, p.value)
        return self

    def to_xml_bytes(self) -> bytes:
        ET.register_namespace("ssv", SSP_SSV_NS)
        ET.register_namespace("ssc", SSP_SSC_NS)
        root = ET.Element(f"{{{SSP_SSV_NS}}}ParameterSet", attrib={"version": "1.0", "name": self.name})
        params_el = ET.SubElement(root, f"{{{SSP_SSV_NS}}}Parameters")
        for p in self._params:
            p_el = ET.SubElement(params_el, f"{{{SSP_SSV_NS}}}Parameter", attrib={"name": p.name})
            # Typed value element with 'value' attribute
            ET.SubElement(p_el, f"{{{SSP_SSV_NS}}}{p.type}", attrib={"value": p.value or ""})

        tree = ET.ElementTree(root)
        ET.indent(tree, space="\t", level=0)
        return ET.tostring(root, encoding="utf-8", xml_declaration=True)


    def build(self, out: str):
        out_path = Path(out)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_bytes(self.to_xml_bytes())
        print(f"Wrote parameter set: {out_path}")