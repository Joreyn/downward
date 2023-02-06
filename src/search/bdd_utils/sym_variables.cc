#include "sym_variables.h"

#include <cmath>
#include "../utils/logging.h"


using namespace std;
using namespace utils;

namespace symbolic {
    SymVariables::SymVariables(const TaskProxy &task_proxy) {
        int numBDDVars=calc_numBDDVars(task_proxy);
        //double the size, because x,x'
        mgr = new Cudd(2*numBDDVars); //TODO cachesize etc if needed

        //initialize variables
        //TODO: determine, whether initialization of size at beginning or use of push_back is faster
        bdd_variables = vector<BDD>(numBDDVars);
        primed_bdd_variables = vector<BDD>(numBDDVars);
        for (int i = 0; i < numBDDVars; i++) {
            bdd_variables[i]=mgr->bddVar(2*i);
            primed_bdd_variables[i]=mgr->bddVar(2*i+1);
        }
        g_log<<"bddVars created"<<endl;


        //init the vector sizes for precondition/effect
        unsigned int numOfNormalVariables = task_proxy.get_variables().size();
        preconditions_vector = vector<vector<BDD>>(numOfNormalVariables);
        effects_vector = vector<vector<BDD>>(numOfNormalVariables);
        biimplication = vector<BDD>(numOfNormalVariables);

        create_index_map(task_proxy); //TODO: this leads to maybe not intended ordering
        g_log<<"printing index map:"<<endl;
        for (auto item:index_map){
            g_log<<item.first<<":"<<item.second<<endl;
        }
        g_log<<endl;

        init_prec_eff(task_proxy);
        g_log<<"init_prec_eff finished"<<endl;

        int id;
        for (VariableProxy variable : task_proxy.get_variables()){
            id = variable.get_id();
            BDD temp = mgr->bddOne();
            for (int i = 0; i<variable.get_domain_size(); i++){
                temp*=(preconditions_vector[id][i]^!effects_vector[id][i]);
            }
            biimplication[variable.get_id()]=temp;
        }


        g_log<<"SymVariables created"<<endl;

    }
    //create the BDDs associated with each value and save them in preconditions/effect
    void SymVariables::init_prec_eff(const TaskProxy &task_proxy) {
        int variable_position; //the position the variable has in the index map
        int variable_id;
        for (VariableProxy variableProxy : task_proxy.get_variables()){
            variable_id = variableProxy.get_id();

            int domain_size = variableProxy.get_domain_size();
            for (int variable_value = 0; variable_value<domain_size;variable_value++){
                preconditions_vector[variable_id].push_back(mgr->bddOne());
                effects_vector[variable_id].push_back(mgr->bddOne());
                vector <bool> bool_vector= convert_to_bool_vector(variable_value, ceil(log2(domain_size)));
                variable_position = index_map[variable_id];
                // TODO: evaluate, whether flipping the bool_vector improves runtime,
                for (bool b : bool_vector){
                    //TODO: maybe swap x,x', by swapping current_index/current_index+1
                    // otherwise keep as is (thus leading to different order)
                    if (b){
                        preconditions_vector[variable_id][variable_value]*= bdd_variables[variable_position];
                        effects_vector[variable_id][variable_value]*= primed_bdd_variables[variable_position];
                    }else{
                        preconditions_vector[variable_id][variable_value]*=!bdd_variables[variable_position];
                        effects_vector[variable_id][variable_value]*=!primed_bdd_variables[variable_position];
                    }
                    variable_position++;
                }
            }
        }
    }

    // TODO: merge with calling function for less loops and stability
    //  as it could lead to unused BDD variables/overlaps/segmentation errors
    void SymVariables::create_index_map(const TaskProxy &task_proxy) {
        int current_index = 0;
        int size;
        VariablesProxy _variables = task_proxy.get_variables();
        for (VariableProxy variable: _variables) {
            index_map[variable.get_id()] = current_index;
            size = ceil(log2(variable.get_domain_size()));
            current_index += size;
        }
    }

    int SymVariables::calc_numBDDVars(const TaskProxy &task_proxy){
        int sum = 0;
        for (VariableProxy variable:task_proxy.get_variables()){
            sum+=ceil(log2(variable.get_domain_size()));
        }
        return sum;
    }

    vector<bool> SymVariables::convert_to_bool_vector(int value, int vector_size) {
        std::vector<bool> bool_vector;
        for (int i = 0; i < vector_size; i++) {
            bool_vector.push_back(value & 1);
            value = value >> 1;
        }
        return bool_vector;
    }

    Cudd *SymVariables::getMgr() const {
        return mgr;
    }

    const vector<BDD> &SymVariables::getVariables() const {
        return bdd_variables;
    }

    const vector<vector<BDD>> &SymVariables::getPreconditionsVector() const {
        return preconditions_vector;
    }

    const vector<vector<BDD>> &SymVariables::getEffectsVector() const {
        return effects_vector;
    }

    const vector<BDD> &SymVariables::getBiimplication() const {
        return biimplication;
    }

    const vector<BDD> &SymVariables::getPrimedBddVariables() const {
        return primed_bdd_variables;
    }

    SymVariables::~SymVariables() = default;
}
