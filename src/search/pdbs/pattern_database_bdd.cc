#include "pattern_database_bdd.h"

#include <queue>
#include <limits>
#include <cmath>
#include <utility>

#include "../utils/math.h"
#include "../utils/logging.h"
#include "../utils/rng.h"
#include "../task_utils/task_properties.h"

using namespace std;
using namespace transition;
using namespace utils;

namespace pdbs {
    PatternDatabaseBDD::PatternDatabaseBDD(
            const TaskProxy &task_proxy,
            const Pattern &pattern,
            symbolic::SymVariables *symVariables,
            const vector<int> &operator_costs,
            bool compute_plan,
            const shared_ptr<utils::RandomNumberGenerator> &rng,
            bool compute_wildcard_plan)
            : pattern(pattern), task_proxy(task_proxy) {
        task_properties::verify_no_axioms(task_proxy);
        task_properties::verify_no_conditional_effects(task_proxy);
        assert(operator_costs.empty() ||
               operator_costs.size() == task_proxy.get_operators().size());
        assert(utils::is_sorted_unique(pattern));
        assert(compute_wildcard_plan == false);
        for (unsigned int var_id = 0; var_id < task_proxy.get_variables().size(); var_id++) {
            domain_sizes_bdd.push_back(ceil(log2(task_proxy.get_variables()[var_id].get_domain_size())));
        }
        num_states = 1;
        for (int pattern_var_id: pattern) {
            VariableProxy var = task_proxy.get_variables()[pattern_var_id];
            if (utils::is_product_within_limit(num_states, var.get_domain_size(),
                                               numeric_limits<int>::max())) {
                num_states *= var.get_domain_size();
            } else {
                cerr << "Given pattern is too large! (Overflow occured): " << endl;
                cerr << pattern << endl;
                utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
            }
        }

        assert(symVariables!= nullptr);
        transition_relation = new TransitionRelation(task_proxy, pattern, symVariables);

        mgr = transition_relation->mgr;

        // Extract the index of zero cost Transition from all defined transitions
        unsigned int index_no_cost = numeric_limits<unsigned int>::max();
        for (unsigned int i = 0; i < transition_relation->transitions.size(); i++) {
            if (transition_relation->transitions[i].cost != 0) {
                continue;
            } else {
                index_no_cost = i;
                break;
            }
        }

        // calc bdd of goal
        Transition goal = Transition(mgr->bddOne(), 0);
        for (const FactProxy &fp: task_proxy.get_goals()) {
            if(binary_search(pattern.begin(), pattern.end(),fp.get_variable().get_id())) {
                goal.bdd *= transition_relation->fact_to_bdd(fp, false);
            }
        }
        //expand goal with 0-cost transitions
        Transition old_states = goal;
        Transition expanded_states = apply_zero_cost_transitions(old_states, index_no_cost);


        //TODO use pointers/initialize variables earlier (result only as local variable here)
        priority_queue<Transition> pq;
        pq.push(expanded_states);

        vector<Transition> pdb_as_bdd;
        pdb_as_bdd.push_back(expanded_states);
        while (!pq.empty()) {
            // TODO: initialize variables earlier, despite warning
            old_states = pq.top();
            pq.pop();
            //make a single bdd out of all the states with the same cost
            while (!pq.empty() && (pq.top().cost == old_states.cost)) {
                old_states.bdd += pq.top().bdd;
                pq.pop();
            }
            pdb_as_bdd.push_back(old_states);

            for (unsigned int index = 0; index < transition_relation->transitions.size(); index++) {
                if (index == index_no_cost) continue;
                expanded_states = apply(transition_relation->transitions[index], old_states);
                if ((expanded_states.bdd * !old_states.bdd) != mgr->bddZero()) {
                    expanded_states = apply_zero_cost_transitions(expanded_states, index_no_cost);
                    pq.push(expanded_states);
                }
            }
        }

        ADD cost;
        ADD constant_max = mgr->constant(numeric_limits<int>::max());
        cost_map_add = constant_max;
        ADD temp_add;
        for (const Transition& trans: pdb_as_bdd) {
            cost = mgr->constant(trans.cost);
            temp_add = trans.bdd.Add().Ite(cost, constant_max);
            cost_map_add = cost_map_add.Minimum(temp_add);
        }
        if (compute_plan) {
            compute_plan_(rng);
        }
    }

    //compute plan using BFS (thus using no rng)
    void PatternDatabaseBDD::compute_plan_(const shared_ptr<utils::RandomNumberGenerator> &rng) {
        struct GeneratedState {
            vector<int> op_ids;
            vector<int> state;
            int h;

            GeneratedState(vector<int> op_ids, vector<int> state, int h)
                    : op_ids(std::move(op_ids)),
                      state(std::move(state)),
                      h(h) {};
        };
        auto comparator = [](const GeneratedState &a, const GeneratedState &b) { return a.h > b.h; };
        priority_queue<GeneratedState, vector<GeneratedState>, decltype(comparator)> pq(comparator);
        task_proxy.get_initial_state().unpack();
        vector<int> initial_state = task_proxy.get_initial_state().get_unpacked_values();
        //TODO task_proxy.get_initial_state().get_unregistered_successor();
        pq.emplace(vector<int>(), initial_state, get_value(initial_state));
        GeneratedState old = pq.top();
        GeneratedState applied = pq.top();
        vector<int> new_state;
        while (!pq.empty()) {
            old = pq.top();
            pq.pop();
            for (OperatorProxy op: task_proxy.get_operators()) {
                //TODO: add operator applyable
                new_state = apply_operator(old.state, op);
                applied = GeneratedState(applied.op_ids, new_state, get_value(new_state));
                applied.op_ids.push_back(op.get_id());
                if (((applied.h + op.get_cost()) == old.h)) {
                    pq.push(applied);
                    if (is_goal(applied.state)) {
                        for (int op_id: applied.op_ids) {
                            wildcard_plan.emplace_back();
                            wildcard_plan.back().emplace_back(op_id);
                        }
                        return;
                    }
                }
            }
        }
    }


    //TODO: rename function, as the conjunction is also included here
    Transition PatternDatabaseBDD::apply(const Transition &transition, const Transition &reached) {
        Transition _reached = reached;
        _reached.bdd = _reached.bdd.SwapVariables(transition_relation->primed_bdd_variables,
                                                  transition_relation->bdd_variables);
        _reached.bdd *= transition.bdd;
        _reached = forget(_reached);

        return Transition(_reached.bdd += reached.bdd, _reached.cost + transition.cost);
    }

    inline Transition &PatternDatabaseBDD::forget(Transition &_reached) {
        //TODO understand temp.ExistAbstract() and Constrain and evaluate whether they would be faster
        for (const BDD &bdd: transition_relation->primed_bdd_variables) {
            _reached.bdd = _reached.bdd.ExistAbstract(bdd);
        }
        return _reached;
    }

    Transition PatternDatabaseBDD::apply_zero_cost_transitions(Transition input_state, unsigned int zero_cost_index) {
        if ((zero_cost_index == numeric_limits<unsigned int>::max())) return input_state;
        Transition new_state = input_state;
        while (true) {
            input_state = new_state;
            new_state = apply(transition_relation->transitions[zero_cost_index], input_state);
            if ((new_state.bdd * !input_state.bdd) == mgr->bddZero()) break;
        }
        return new_state;
    }

    //TODO implement with ADD
    int PatternDatabaseBDD::get_value(const vector<int> &state) const {
        vector<int> converted;
        int value;
        for (unsigned int var_id = 0; var_id < state.size(); var_id++) {
            value = state[var_id];
            for (int i = 0; i < domain_sizes_bdd[var_id]; i++) {
                converted.push_back(value & 1); //unprimed
                converted.push_back(value & 1); //primed
                value = value >> 1;
            }
        }
        ADD eval_add = cost_map_add.Eval(converted.data());
        return static_cast<int>(Cudd_V(eval_add.getRegularNode()));
    }

    const Pattern &PatternDatabaseBDD::get_pattern() const {
        return pattern;
    }

    int PatternDatabaseBDD::get_size() const {
        return num_states;
    }

    vector<vector<OperatorID>> &&PatternDatabaseBDD::extract_wildcard_plan() {
        return std::move(wildcard_plan);
    }

    //TODO: probably extremely inefficient
    double PatternDatabaseBDD::compute_mean_finite_h() const {
        double sum = 0;
        int valid_states = 1;
        vector<int> domain_sizes;
        for (VariableProxy var: task_proxy.get_variables()) {
            if (std::any_of(pattern.begin(), pattern.end(), [&var](int i) { return i == var.get_id(); })) {
                valid_states *= var.get_domain_size();
                domain_sizes[var.get_id()] = var.get_domain_size();
            } else {
                domain_sizes[var.get_id()] = 1;
            }
        }
        vector<int> current_state(task_proxy.get_variables().size(), 0);
        int last = current_state.size();
        int i = 1;
        while (i > -1) {
            sum += get_value(current_state);
            i = last;
            while (current_state[i]++ >= domain_sizes[i]) {
                current_state[i] = 0;
                i--;
                if (i < 0) {
                    break;
                }
            }
        }
        return sum / valid_states;
    };

    //copied from pattern_database_explicit
    bool PatternDatabaseBDD::is_operator_relevant(const OperatorProxy &op) const {
        for (EffectProxy effect: op.get_effects()) {
            int var_id = effect.get_fact().get_variable().get_id();
            if (binary_search(pattern.begin(), pattern.end(), var_id)) {
                return true;
            }
        }
        return false;
    }

    bool PatternDatabaseBDD::operator_applyable(const vector<int> &state, OperatorProxy op) const {
        for (auto prec: op.get_preconditions()) {
            if ((state[prec.get_variable().get_id()] != prec.get_value()) &&
                binary_search(pattern.begin(), pattern.end(), prec.get_variable().get_id()))
                return false;
        }
        return true;
    }

    vector<int> PatternDatabaseBDD::apply_operator(const vector<int> &state, OperatorProxy op) const {
        if(!operator_applyable(state,op)) return state;
        vector<int> new_state = state;
        for (EffectProxy eff: op.get_effects()) {
            if (any_of(pattern.begin(), pattern.end(),
                       [&eff](int i) { return i == eff.get_fact().get_variable().get_id(); })) {
                new_state[eff.get_fact().get_variable().get_id()] = eff.get_fact().get_value();
            }
        }
        return new_state;
    }

    bool PatternDatabaseBDD::is_goal(vector<int> state) {
        for (FactProxy goal: task_proxy.get_goals()) {
            if (any_of(pattern.begin(), pattern.end(), [&goal](int i) { return (i == goal.get_value()); }) &&
                state[goal.get_variable().get_id()] != goal.get_value())
                return false;
        }
        return true;
    }
}
