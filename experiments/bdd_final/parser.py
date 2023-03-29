#! /usr/bin/env python

import logging
import re

from lab.parser import Parser


class CommonParser(Parser):
    def add_repeated_pattern(
        self, name, regex, file="run.log", required=False, type=int
    ):
        def find_all_occurences(content, props):
            matches = re.findall(regex, content)
            if required and not matches:
                logging.error(f"Pattern {regex} not found in file {file}")
            props[name] = [type(m) for m in matches]

        self.add_function(find_all_occurences, file=file)

    def add_sum_pattern(
        self, name, regex, file="run.log", required=False, type=int
    ):
        def sum_all_occurences(content, props):
            matches = re.findall(regex, content)
            if required and not matches:
                logging.error(f"Pattern {regex} not found in file {file}")
            props[name]=0
            for m in matches:
                props[name] += type(m)

        self.add_function(sum_all_occurences, file=file)

    def add_bottom_up_pattern(
        self, name, regex, file="run.log", required=False, type=int
    ):
        def search_from_bottom(content, props):
            reversed_content = "\n".join(reversed(content.splitlines()))
            match = re.search(regex, reversed_content)
            if required and not match:
                logging.error(f"Pattern {regex} not found in file {file}")
            if match:
                props[name] = type(match.group(1))

        self.add_function(search_from_bottom, file=file)


def main():
    parser = CommonParser()
    #cpdbs
    parser.add_pattern(
        "cpdbs_number_of_patterns",
        "Canonical PDB heuristic number of patterns: (\d+)",
        required=False,
        type=int
    )
    parser.add_pattern(
        "cpdbs_total_pdb_size",
        "Canonical PDB heuristic total PDB size: (\d+)",
        required=False,
        type=int
    )
    parser.add_pattern(
        "cpdbs_average_pdb_size",
        "Canonical PDB heuristic average PDB size: (\d+)",
        required=False,
        type=int
    )
    parser.add_pattern(
        "cpdbs_computation_time",
        "Canonical PDB heuristic computation time: (.+)s",
        required=False,
        type=float
    )

    # pdb
    parser.add_pattern(
        "pdb_number_of_variables",
        "PDB Heuristic number of variables: (\d+)",
        required=False,
        type=int
    )
    parser.add_pattern(
        "pdb_size",
        "PDB Heuristic PDB size: (\d+)",
        required=False,
        type=int
    )
    parser.add_pattern(
        "pdb_computation_time",
        "PDB Heuristic computation time: (.+)s",
        required=False,
        type=float
    )
    parser.parse()


if __name__ == "__main__":
    main()
