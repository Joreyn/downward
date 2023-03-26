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
    parser.add_sum_pattern(
        "PDB_create_time",
        r"\[t=(.+)s, \d+ KB\] PDB: creating...",
        type=float,
    )
    parser.add_sum_pattern(
        "PDB_finished_time",
        r"\[t=(.+)s, \d+ KB\] PDB: size=",
        type=float,
    )
    parser.add_sum_pattern(
        "PDB_before_memory",
        r"\[t=.+s, (\d+) KB\] PDB: creating...",
        type=int,
    )
    parser.add_sum_pattern(
        "PDB_after_memory",
        r"\[t=.+s, (\d+) KB\] PDB: size=",
        type=int,
    )
    parser.add_sum_pattern(
        "PDB_rel_size",
        r"\[t=.+s, \d+ KB\] PDB: size=(\d+)",
        type=int,
    )
    parser.parse()


if __name__ == "__main__":
    main()
