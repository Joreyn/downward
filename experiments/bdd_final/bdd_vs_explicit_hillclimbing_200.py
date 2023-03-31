#! /usr/bin/env python

import os
import shutil

import project
from custom_filter import CustomFilter

REPO = project.get_repo_base()
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
SCP_LOGIN = "fahmat01@login-infai.scicore.unibas.ch"
REMOTE_REPOS_DIR = "/infai/seipp/projects"
# If REVISION_CACHE is None, the default ./data/revision-cache is used.
REVISION_CACHE = os.environ.get("DOWNWARD_REVISION_CACHE")

if project.REMOTE:
    SUITE = project.SUITE_OPTIMAL_STRIPS
    #SUITE = ["depot:p01.pddl", "grid:prob01.pddl", "gripper:prob01.pddl"]
    ENV = project.BaselSlurmEnvironment(email="m.fahrni@stud.unibas.ch", partition="infai_2",
    # paths obtained via:
    # module purge
    # module -q load CMake/3.15.3-GCCcore-8.3.0
    # module -q load GCC/8.3.0
    # echo $PATH
    # echo $LD_LIBRARY_PATH
    setup='export PATH=/scicore/soft/apps/binutils/2.32-GCCcore-8.3.0/bin:/scicore/soft/apps/CMake/3.15.3-GCCcore-8.3.0/bin:/scicore/soft/apps/cURL/7.66.0-GCCcore-8.3.0/bin:/scicore/soft/apps/bzip2/1.0.8-GCCcore-8.3.0/bin:/scicore/soft/apps/ncurses/6.1-GCCcore-8.3.0/bin:/scicore/soft/apps/GCCcore/8.3.0/bin:/infai/fahmat01/repos/bin:/infai/fahmat01/local:/export/soft/lua_lmod/centos7/lmod/lmod/libexec:/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:$PATH\nexport LD_LIBRARY_PATH=/scicore/soft/apps/binutils/2.32-GCCcore-8.3.0/lib:/scicore/soft/apps/cURL/7.66.0-GCCcore-8.3.0/lib:/scicore/soft/apps/bzip2/1.0.8-GCCcore-8.3.0/lib:/scicore/soft/apps/zlib/1.2.11-GCCcore-8.3.0/lib:/scicore/soft/apps/ncurses/6.1-GCCcore-8.3.0/lib:/scicore/soft/apps/GCCcore/8.3.0/lib64:/scicore/soft/apps/GCCcore/8.3.0/lib')
else:
    SUITE = ["depot:p01.pddl", "grid:prob01.pddl", "gripper:prob01.pddl", "tetris-opt14-strips:p03-4.pddl"]
    ENV = project.LocalEnvironment(processes=2)
CONFIGS = [
    (f"cpdbs_hill_200_exp", ["--search", "astar(cpdbs(pdb_type=explicit, patterns=hillclimbing(max_generated_patterns=200, pdb_type=explicit)))"]),
    (f"cpdbs_hill_200_bdd", ["--search", "astar(cpdbs(pdb_type=bdd, patterns=hillclimbing(max_generated_patterns=200, pdb_type=bdd)))"])
]
BUILD_OPTIONS = []
DRIVER_OPTIONS = ["--overall-time-limit", "30m"]
REVS = [
    ("86c31ffa58aa19037ffd2cce9ff7c8380a408a21", "version:31.03.23"),
]
ATTRIBUTES = [
    "cost",
    "coverage",
    "error",
    "evaluations",
    "expansions",
    "expansions_until_last_jump",
    "generated",
    "memory",
    "planner_memory",
    "planner_time",
    "quality",
    "run_dir",
    "score_evaluations",
    "score_expansions",
    "score_generated",
    "score_memory",
    "score_search_time",
    "score_total_time",
    "search_time",
    "total_time",
    "search_start_time",
    "search_start_memory",
    "h_values",
    "initial_h_values",
    project.EVALUATIONS_PER_TIME,
    project.CPDBS_COMPUTATION_TIME,
    project.CPDBS_NUMBER_OF_PATTERNS,
    project.CPDBS_TOTAL_PDB_SIZE,
    project.CPDBS_AVERAGE_PDB_SIZE

]

exp = project.FastDownwardExperiment(environment=ENV, revision_cache=REVISION_CACHE)
for config_nick, config in CONFIGS:
    algo_name = f"{config_nick}"
    exp.add_algorithm(
        algo_name,
        REPO,
        REVS[0][0],
        config,
        build_options=BUILD_OPTIONS,
        driver_options=DRIVER_OPTIONS,
    )
exp.add_suite(BENCHMARKS_DIR, SUITE)

exp.add_parser(exp.EXITCODE_PARSER)
exp.add_parser(exp.TRANSLATOR_PARSER)
exp.add_parser(exp.SINGLE_SEARCH_PARSER)
exp.add_parser(project.DIR / "parser.py")
exp.add_parser(exp.PLANNER_PARSER)

exp.add_step("build", exp.build)
exp.add_step("start", exp.start_runs)
exp.add_fetcher(name="fetch")

if not project.REMOTE:
    exp.add_step("remove-eval-dir", shutil.rmtree, exp.eval_dir, ignore_errors=True)
    project.add_scp_step(exp, SCP_LOGIN, REMOTE_REPOS_DIR)

project.add_absolute_report(
    exp, attributes=ATTRIBUTES
)

exp.add_report(
    project.ScatterPlotReport(
        attributes=[project.PDB_COMPUTATION_TIME],
        show_missing=True,
        title="pdb computation time using greedy pattern generator",
        get_category=project.unit_as_category,
        scale="log",
        relative=False,
    ),
    name="time_hill200"
)

exp.add_report(
    project.ScatterPlotReport(
        attributes=[project.CPDBS_TOTAL_PDB_SIZE],
        show_missing=True,
        title="pdb computation time using greedy pattern generator",
        get_category=project.unit_as_category,
        scale="log",
        relative=False
    ),
    name="size_hill200"
)
exp.add_report(
    project.ScatterPlotReport(
        attributes=[project.CPDBS_TOTAL_PDB_SIZE],
        show_missing=True,
        title="pdb computation time using greedy pattern generator",
        get_category=project.unit_as_category,
        scale="log",
        relative=True
    ),
    name="size_hill200_relative"
)

exp.run_steps()
