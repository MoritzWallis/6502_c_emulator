import sys

STANDARD_SOURCE_FILE = "source.asm"


class Assembler:
    source_code = None
    variables = None

    # Remove comments from the source code
    @staticmethod
    def clean_comments():
        Assembler.source_code = "\n".join(
            line.split(";")[0].strip() for line in source_code.split("\n")
        )

    # Extract variables and their values and store them in a dictionary
    @staticmethod
    def process_variables():
        working_code = Assembler.source_code.split("\n")
        Assembler.variables = {}

        for i, line in enumerate(working_code):
            if '=' in line:
                variable, value = line.split('=')
                Assembler.variables[variable.strip()] = value.strip()
                working_code[i] = ""
        Assembler.source_code = "\n".join(working_code)

    # Remove empty lines and strip unnecessary spaces
    @staticmethod
    def clean_spaces():
        working_code = Assembler.source_code.split("\n")
        Assembler.source_code = "\n".join([line.strip() for line in working_code if line.strip()])

    # Get and get rid of the origin and export point
    @staticmethod
    def get_origin_and_export():
        working_code = Assembler.source_code.split("\n")
        origin = "NOT FOUND"
        export = "NOT FOUND"
        for i, line in enumerate(working_code):
            if line.startswith((".org", ".ORG")):
                origin = line[4:].strip()
                working_code[i] = ""
            elif line.startswith((".export", ".export")):
                export = line[7:].strip()
                working_code[i] = ""


        Assembler.source_code = "\n".join(working_code)

        return origin, export


    @staticmethod
    def process_Labels():
        working_code = Assembler.source_code.split("\n")
        Assembler.labels = {}

        for i, line in enumerate(working_code):
            if ':' in line:
                label = line.split(':')[0].strip()
                Assembler.labels[label] = i
                working_code[i] = line.split(':')[1].strip()
        Assembler.source_code = "\n".join(working_code)

    # Assemble the source code - main function
    @staticmethod
    def assemble(_source_code):
        print("Assembling")
        Assembler.source_code = _source_code
        Assembler.clean_comments()
        Assembler.process_variables()
        origin_export = Assembler.get_origin_and_export()
        Assembler.clean_spaces()


        # Assembler.process_Labels()

        print("Done assembling")
        return origin_export, Assembler.source_code


if __name__ == "__main__":
    print("Assembler Activated")
    None if len(sys.argv) == 2 else sys.argv.append("wosmon.asm")  # For testing purposes

    source_file = sys.argv[1] if len(sys.argv) == 2 else STANDARD_SOURCE_FILE
    print(f"Assembling file '{source_file}'")

    try:
        with open(source_file) as f:
            source_code = f.read()
    except FileNotFoundError:
        print(f"Error: file '{source_file}' not found")
        sys.exit(1)

    origin_export, assembled_code = Assembler.assemble(source_code)

    print(f"Origin: {origin_export[0]}\nexport: {origin_export[1]}")

    output_file = source_file.replace(".asm", ".bin")
    with open(output_file, "w") as f:
        f.write(assembled_code)
