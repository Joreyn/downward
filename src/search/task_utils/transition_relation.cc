#include "transition_relation.h"

#include "math.h"

using namespace transitionRelation;


namespace transitionRelation {
//TODO test
void init(TaskProxy &task_proxy){
    //initialize CUDD
    //TODO: allocate memory for CUDD (cache size etc. (possibly just copy from other file))
    int numBDDVars=calc_numBDDVars(task_proxy);
    //place mgr to another function(cudd_init or something) as this works only for binary variables + cleaner that way
    mgr = new Cudd (numBDDVars);
    for (int i = 0; i<numBDDVars;i++){
        bdd_variables.push_back(mgr->bddVar(i));
    }

    //Initialize the index_map
    init_index_map(task_proxy);

    //Create a transition relation for all operators and save them in vector transRelations
    init_transRelations(task_proxy);

    GoalsProxy goals = task_proxy.get_goals();
}

//TODO test
void init_index_map(const TaskProxy &task_proxy) {
    int current_index=0;
    int size;
    VariablesProxy variables = task_proxy.get_variables();
    for (VariableProxy variable:variables){
        index_map[variable.get_id()]=current_index;
        size=ceil(log2(variable.get_domain_size()));
        current_index+=2*size;
    }
}

void init_transRelations(const TaskProxy &task_proxy) {
    OperatorsProxy operators=task_proxy.get_operators();
    //TODO: allocate memory for transRelations here for more speed
    for (OperatorProxy o:operators){
        int cost = o.get_cost();
        BDD test=mgr->bddOne();
        for (EffectProxy eff:o.get_effects()){
            //in symvariables all the bdd are saved in a vector
            // (leading to std::vector<std::vector<int>> bdd_index_pre, bdd_index_eff;)
            test*=convert_fact_to_bdd(eff.get_fact(), true);
        }
        for (FactProxy prec :o.get_preconditions()){
            test*=convert_fact_to_bdd(prec, false);
        }
        transRelations.emplace_back(test,cost,o.get_name());
    }
}

//TODO: make more efficient, especially for loop
// (possibly initialize a map, which contains all the val -> [bool], thus the calculation only needed once)
BDD convert_fact_to_bdd(FactProxy fact, bool is_effect){
    BDD bdd=mgr->bddOne();
    int index = index_map[fact.get_variable().get_id()];
    if (is_effect){
        index++;
    }
    int j = 0;
    int num_of_bdd_variables=ceil(log2(fact.get_variable().get_domain_size()));
    std::vector<bool> vector = convert_to_bool_vector(fact.get_value(),num_of_bdd_variables);
    for (int i = index; i<num_of_bdd_variables+index; i+=2){
        //TODO make cleaner with inline operator or something
        if (vector[j]){
            bdd*=bdd_variables[i];
        }else{
            bdd*=!bdd_variables[i];
        }
    }
    return bdd;
}

std::vector<bool> convert_to_bool_vector(int value, int vector_size){
    //TODO: make efficient
    std::vector<bool> bool_vector;
    int max = (int) pow(2,vector_size-1);
    for (int i = 0; i<vector_size; i++){
        bool_vector[i]=max<=value;
        value/=2;
    }
    return bool_vector;
}

int calc_numBDDVars(TaskProxy &task_proxy){
    int sum = 0;
    for (VariableProxy variable:task_proxy.get_variables()){
        sum+=ceil(log2(variable.get_domain_size()));
    }
    return 2*sum; //double the sum, because x,x'
}
}