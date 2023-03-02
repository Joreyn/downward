#include "pattern_database_bdd.h"

#include <queue>
#include <limits>
#include <cmath>

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


        transition_relation = new TransitionRelation(task_proxy, (vector<int>) pattern, symVariables);
        g_log << "TransitionRelation finished" << endl;

        mgr = transition_relation->mgr;

        // calc bdd of goal
        Transition goal = Transition(mgr->bddOne(), 0);
        for (const FactProxy &fp: task_proxy.get_goals()) {
            goal.bdd *= transition_relation->fact_to_bdd(fp, false);
        }

        // TODO: (after merging same cost Transitions)
        //  change index no cost to int, as there will be only one after merging same cost transitions

        // Extract the index of zero cost Transition from all defined transitions
        vector<unsigned int> index_no_cost;
        vector<unsigned int> index_with_cost;
        for (unsigned int i = 0; i < transition_relation->transitions.size(); i++) {
            if (transition_relation->transitions[i].cost != 0) {
                index_with_cost.push_back(i);
            } else {
                index_no_cost.push_back(i);
            }
        }

        //TODO use pointers/initialize variables earlier (result only as local variable here)
        priority_queue<Transition> pq;
        pq.push(goal);
        //TODO edit break condition
        vector<Transition> pdb_as_bdd;
        while (!pq.empty()) {
            // TODO: initialize variables earlier, despite warning
            Transition top = pq.top();
            Transition currently_evaluated = pq.top();
            pq.pop();

            //make a single bdd out of all the states with the same cost
            while (!pq.empty() && (pq.top().cost == currently_evaluated.cost)) {
                currently_evaluated.bdd += pq.top().bdd;
                pq.pop();
            }
            //evaluate the 0 cost transitions first
            // (it is also directly added for the not zero cost transition relations)
            while (true) {
                //TODO: since operations with same cost are already merged, replace loop with "if"
                for (unsigned int i: index_no_cost) {
                    currently_evaluated = top;
                    top = apply(transition_relation->transitions[i], top);
                }
                //TODO use LEQ or similar instead of checking ...==mgr->bddZero()
                if ((top.bdd * !currently_evaluated.bdd) == mgr->bddZero()) break;
            }
            for (unsigned int i: index_with_cost) {
                top = currently_evaluated;
                top = apply(transition_relation->transitions[i], top);
                if ((top.bdd * !currently_evaluated.bdd) != mgr->bddZero()) {
                    pq.push(top);
                }
            }
            pdb_as_bdd.push_back(currently_evaluated);
        }
        g_log << "creating ADD" << endl;

        ADD cost;
        ADD constant_max = mgr->constant(numeric_limits<int>::max());
        cost_map_add = constant_max;
        ADD temp_add;
        g_log << pdb_as_bdd.size() << endl;
        for (const Transition &trans: pdb_as_bdd) {
            cost = mgr->constant(trans.cost);
            temp_add = trans.bdd.Add().Ite(cost, constant_max);
            cost_map_add = cost_map_add.Minimum(temp_add);
        }
        g_log << "pdb created" << endl;

        //TODO properly handle 0-cost operations
        if (compute_plan) {
            foo(rng);

        }
    }

    void PatternDatabaseBDD::foo(const shared_ptr<utils::RandomNumberGenerator> &rng) {
        //vector<int> generating_op_ids;
        task_proxy.get_initial_state().unpack();
        vector<int> old_state = task_proxy.get_initial_state().get_unpacked_values();
        vector<int> new_state = task_proxy.get_initial_state().get_unpacked_values();
        int op_size = task_proxy.get_operators().size();
        bool change_happened = true;
        int random_index = 0;
        int current_index = 0;
        OperatorsProxy operators = task_proxy.get_operators();
        while (get_value(old_state) != 0 && change_happened) {
            change_happened = false;
            random_index = rng->random(op_size);
            for (int i = 0; i < op_size; i++) {
                current_index = (random_index + i) % op_size;
                if (!is_operator_relevant(operators[current_index])||!operator_applyable(old_state, operators[current_index]))continue;
                new_state = apply_operator(old_state, operators[current_index]);
                if (get_value(new_state) + operators[current_index].get_cost() == get_value(old_state)){
                    //generating_op_ids.push_back(operators[current_index].get_id());
                    wildcard_plan.emplace_back();
                    wildcard_plan.back().emplace_back(current_index);
                    old_state = new_state;
                    break;
                }
            }

        }
    }


    //TODO: rename function, as the conjunction is also included here
    Transition PatternDatabaseBDD::apply(const Transition &transition, const Transition &reached) {
        //g_log << "apply" << endl;
        Transition _reached = reached;
        //TODO understand _reached.bdd.AdjPermuteX() and evaluate whether it would be faster
        _reached.bdd = _reached.bdd.SwapVariables(transition_relation->primed_bdd_variables,
                                                  transition_relation->bdd_variables);
        _reached.bdd *= transition.bdd;
        _reached = forget(_reached);

        if (transition.cost == 0) {
            g_log << "cost zero transition" << endl;
            return Transition(_reached.bdd += reached.bdd, _reached.cost + transition.cost);
        }
        //TODO: evaluate, whether to use _reached.bdd+=reached.bdd or _reached.bdd
        return Transition(_reached.bdd += reached.bdd, _reached.cost + transition.cost);
    }

    inline Transition &PatternDatabaseBDD::forget(Transition &_reached) {
        //TODO understand temp.ExistAbstract() and Constrain and evaluate whether they would be faster
        for (const BDD &bdd: transition_relation->primed_bdd_variables) {
            _reached.bdd = _reached.bdd.ExistAbstract(bdd);
        }
        return _reached;
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
        return (int) Cudd_V(eval_add.getRegularNode());
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
            if (state[prec.get_variable().get_id()] != prec.get_value() &&
                any_of(pattern.begin(), pattern.end(), [&prec](int i) { return prec.get_variable().get_id() == i; }))
                return false;
        }
        return true;
    }

    vector<int> PatternDatabaseBDD::apply_operator(vector<int> state, OperatorProxy op) const {
        for(EffectProxy eff:op.get_effects()){
            //if (any_of(pattern.begin(), pattern.end(),[&eff](int i) {return i==eff.get_fact().get_variable().get_id();})){
            state[eff.get_fact().get_variable().get_id()]=eff.get_fact().get_value();
            //}
        }
        return state;
    }
}
