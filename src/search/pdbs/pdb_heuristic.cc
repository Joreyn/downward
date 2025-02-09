#include "pdb_heuristic.h"

#include "pattern_database.h"
#include "pattern_generator.h"

#include "../plugins/plugin.h"

#include <limits>
#include <memory>

using namespace std;

namespace pdbs {
shared_ptr<PatternDatabase> get_pdb_from_options(const shared_ptr<AbstractTask> &task,
                                                 const plugins::Options &opts) {
    shared_ptr<PatternGenerator> pattern_generator =
        opts.get<shared_ptr<PatternGenerator>>("pattern");
    PatternInformation pattern_info = pattern_generator->generate(task);
    return pattern_info.get_pdb(opts.get<PDBType>("pdb_type"));
}

PDBHeuristic::PDBHeuristic(const plugins::Options &opts)
    : Heuristic(opts),
      pdb(get_pdb_from_options(task, opts)) {
}

int PDBHeuristic::compute_heuristic(const State &ancestor_state) {
    State state = convert_ancestor_state(ancestor_state);
    int h = pdb->get_value(state.get_unpacked_values());
    if (h == numeric_limits<int>::max())
        return DEAD_END;
    return h;
}

static shared_ptr<Heuristic> _parse(OptionParser &parser) {
    {
        parser.document_synopsis("Pattern database heuristic", "TODO");

        parser.add_option<shared_ptr<PatternGenerator>>(
            "pattern",
            "pattern generation method",
            "greedy()");
        Heuristic::add_options_to_parser(parser);

        vector<pair<string, string>> enum_info;
        enum_info.emplace_back("explicit", "explicit PDB");
        enum_info.emplace_back("bdd", "BDD PDB");
        enum_info.emplace_back("evmdd", "EVMDD PDB");
        parser.add_enum_option<PDBType>(
            "pdb_type",
            enum_info,
            "PDB type");

        parser.document_language_support("action costs", "supported");
        parser.document_language_support("conditional effects", "not supported");
        parser.document_language_support("axioms", "not supported");

        parser.document_property("admissible", "yes");
        parser.document_property("consistent", "yes");
        parser.document_property("safe", "yes");
        parser.document_property("preferred operators", "no");
    }
    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    return make_shared<PDBHeuristic>(opts);
}

static Plugin<Evaluator> _plugin("pdb", _parse, "heuristics_pdb");
}
