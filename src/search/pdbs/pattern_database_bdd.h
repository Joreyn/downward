#ifndef PDBS_PATTERN_DATABASE_BDD_H
#define PDBS_PATTERN_DATABASE_BDD_H

#include "pattern_database.h"
#include "../bdd_utils/transition_relation.h"

using namespace transition;
namespace pdbs {
class PatternDatabaseBDD : public PatternDatabase {
    //TODO: where add grouping and reordering?
    // http://web.mit.edu/sage/export/tmp/y/usr/share/doc/polybori/cudd/node3.html#SECTION000316000000000000000
    TransitionRelation* transition_relation;
    Pattern pattern;
    Cudd* mgr;
    ADD cost_map_add;
    vector<int> domain_sizes_bdd; //Needed for
    int num_states;
    int num_nodes;
    const TaskProxy &task_proxy;
    vector<vector<OperatorID>> wildcard_plan;
public:
    PatternDatabaseBDD(
        const TaskProxy &task_proxy,
        const Pattern &pattern,
        symbolic::SymVariables* symVariables = nullptr,
        const std::vector<int> &operator_costs = std::vector<int>(),
        bool compute_plan = false,
        const std::shared_ptr<utils::RandomNumberGenerator> &rng = nullptr,
        bool compute_wildcard_plan = false);
    ~PatternDatabaseBDD() override = default;
    int get_value(const std::vector<int> &state) const override;
    const Pattern &get_pattern() const override;
    int get_size() const override;
    int get_rel_size() const override;
    std::vector<std::vector<OperatorID>> && extract_wildcard_plan() override;
    double compute_mean_finite_h() const override;
    bool is_operator_relevant(const OperatorProxy &op) const override;

private:
    Transition apply(const transition::Transition &transition, const Transition& goal);

    inline Transition &forget(Transition &_reached); //TODO is inline better?
    void compute_plan_(const shared_ptr<utils::RandomNumberGenerator>& rng);

    bool operator_applyable(const vector<int> &state, OperatorProxy op) const;

    vector<int> apply_operator(const vector<int>& state, OperatorProxy op) const;

    Transition apply_zero_cost_transitions(Transition input_state, unsigned int zero_cost_index);

    bool is_goal(vector<int> state);
};
}

#endif
