"""
def mark_broken(lines, idx):
    lines[idx] = lines[idx].append("\t<-- SOMETHINGS BROKEN :(")
    return lines
"""


import re
class List(list):
    def extract_data(self):
        good_lines, misfits, indices = [], [], []

        for idx, org_line in enumerate(self):
            line = org_line.split(" ")
            opc = line[0]
            inst = line[2].strip("\n")
            desc = "NO DESCRIPTION"
            if len(line) > 3:
                desc = " ".join(line[3:]).strip("\n")

            inst_split = inst.split("-")
            nenm = inst_split[0]
            mode = "IMP"
            if len(inst_split) > 1:
                mode = inst_split[1]

            if mode == "a":
                mode = "IMP"

            """"
            byte opc,
            const std::string &name = "NOT NAMED",
            byte(Dodgy6502::*addr_mode)() = nullptr,
            byte(Dodgy6502::*implementation)() = nullptr,
            byte cycles = 0,
            const std::string &description = "NO DESCRIPTION"
            """

            final = f'add_instruction({opc}, "{nenm}", &Dodgy6502::{mode}, &Dodgy6502::{inst_split[0]}, 0, "0x{line[1]} {inst}: {desc}");\n'
            print(final)
            good_lines.append(final)



        return good_lines, misfits, indices

1

if __name__ == "__main__":
    with open("instruction_info_fixed", "r+") as f:
        lines = List(f.readlines())  # just having fun ofcourse
        good_lines, misfits, indices = lines.extract_data()
        # with open("instructions.cpp", "w") as f:
        #     f.write("# include \"6502v2.h\"\n#include \"instructions.h\"\n\n")
        #     f.writelines(good_lines)
