

class Variable:
    """
    name;interpolation_method;time_0,var_1.0;t1,var_1.2\nname,inter....
    """

    def __init__(self, name, interpolation, series):
        self.name = name
        self.interpolation = interpolation
        self.series: list[list[float, float]] = series

    def start_value(self):
        return self.series[0][1]

    @staticmethod
    def from_str(string: str):
        parts = string.split(";")
        coordinates = parts[2:]

        def f(s: str):
            return [float(x.strip()) for x in s.split(",")]

        coordinates = [f(x) for x in coordinates]
        return Variable(parts[0], parts[1], coordinates)

    def to_str(self):
        values = ";".join([f"{x[0]},{x[1]}" for x in self.series])
        return f"{self.name};{self.interpolation};{values}"


class Variables:
    @staticmethod
    def from_string(string: str):
        return [Variable.from_str(x) for x in string.split("\n")]

    @staticmethod
    def to_string(variables: list[Variable]):
        return "\n".join([x.to_str() for x in variables])