#! /usr/bin/env python

import os
import shutil

import project


REPO = project.get_repo_base()
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
SCP_LOGIN = "fahmat01@login-infai.scicore.unibas.ch"
REMOTE_REPOS_DIR = "/infai/seipp/projects"
# If REVISION_CACHE is None, the default ./data/revision-cache is used.
REVISION_CACHE = os.environ.get("DOWNWARD_REVISION_CACHE")

if project.REMOTE:
    #SUITE = project.SUITE_OPTIMAL_STRIPS
    SUITE = ["depot:p01.pddl", "grid:prob01.pddl", "gripper:prob01.pddl"]
    ENV = project.BaselSlurmEnvironment(email="m.fahrni@stud.unibas.ch", partition="infai_2",
    # paths obtained via:
    # module purge
    # module -q load CMake/3.15.3-GCCcore-8.3.0
    # module -q load GCC/8.3.0
    # echo $PATH
    # echo $LD_LIBRARY_PATH
    setup='export PATH=/scicore/soft/apps/binutils/2.32-GCCcore-8.3.0/bin:/scicore/soft/apps/CMake/3.15.3-GCCcore-8.3.0/bin:/scicore/soft/apps/cURL/7.66.0-GCCcore-8.3.0/bin:/scicore/soft/apps/bzip2/1.0.8-GCCcore-8.3.0/bin:/scicore/soft/apps/ncurses/6.1-GCCcore-8.3.0/bin:/scicore/soft/apps/GCCcore/8.3.0/bin:/infai/fahmat01/repos/bin:/infai/fahmat01/local:/export/soft/lua_lmod/centos7/lmod/lmod/libexec:/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:$PATH\nexport LD_LIBRARY_PATH=/scicore/soft/apps/binutils/2.32-GCCcore-8.3.0/lib:/scicore/soft/apps/cURL/7.66.0-GCCcore-8.3.0/lib:/scicore/soft/apps/bzip2/1.0.8-GCCcore-8.3.0/lib:/scicore/soft/apps/zlib/1.2.11-GCCcore-8.3.0/lib:/scicore/soft/apps/ncurses/6.1-GCCcore-8.3.0/lib:/scicore/soft/apps/GCCcore/8.3.0/lib64:/scicore/soft/apps/GCCcore/8.3.0/lib')
else:
    SUITE = ["depot:p01.pddl", "grid:prob01.pddl", "gripper:prob01.pddl"]
    ENV = project.LocalEnvironment(processes=2)
CONFIGS = [
    (f"greedy_explicit", ["--search", "astar(pdb(greedy(pdb_type=explicit),pdb_type=explicit))"]),
    (f"greedy_bdd", ["--search", "astar(pdb(greedy(pdb_type=bdd),pdb_type=bdd))"]),
    (f"cegar_explicit", ["--search", "astar(pdb(cegar_pattern(pdb_type=explicit),pdb_type=explicit))"]),
    (f"cegar_bdd", ["--search", "astar(pdb(cegar_pattern(pdb_type=bdd),pdb_type=bdd))"]),
    (f"cpds_explicit", ["--search", "astar(cpdbs(pdb_type=explicit, patterns=combo(pdb_type=explicit)))"]),
    (f"cpds_bdd", ["--search", "astar(cpdbs(pdb_type=bdd, patterns=combo(pdb_type=bdd)))"])
]
BUILD_OPTIONS = []
DRIVER_OPTIONS = ["--overall-time-limit", "30m"]
REVS = [
    ("1f67b621daa0f33670753106d1a76d2752af928d", "test cpdbs, version:23.02.23"),
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
    project.EVALUATIONS_PER_TIME,
]

exp = project.FastDownwardExperiment(environment=ENV, revision_cache=REVISION_CACHE)
for config_nick, config in CONFIGS:
    for rev, rev_nick in REVS:
        algo_name = f"{rev_nick}:{config_nick}" if rev_nick else config_nick
        exp.add_algorithm(
            algo_name,
            REPO,
            rev,
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
    exp, attributes=ATTRIBUTES, filter=[project.add_evaluations_per_time]
)

attributes = ["expansions"]
pairs = [
    ("20.06:01-cg", "20.06:02-ff"),
]
suffix = "-rel" if project.RELATIVE else ""
for algo1, algo2 in pairs:
    for attr in attributes:
        exp.add_report(
            project.ScatterPlotReport(
                relative=project.RELATIVE,
                get_category=None if project.TEX else lambda run1, run2: run1["domain"],
                attributes=[attr],
                filter_algorithm=[algo1, algo2],
                filter=[project.add_evaluations_per_time],
                format="tex" if project.TEX else "png",
            ),
            name=f"{exp.name}-{algo1}-vs-{algo2}-{attr}{suffix}",
        )

exp.run_steps()
