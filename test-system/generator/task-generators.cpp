#include "task-generators.h"

std::vector<Task> generate_nested_pip_prop_pip(float u_total) {
    return generate_nested(u_total, Subtask::inherited, Subtask::propagated, Subtask::inherited);
}
std::vector<Task> generate_nested_pip_prop_prop(float u_total) {
    return generate_nested(u_total, Subtask::inherited, Subtask::propagated, Subtask::propagated);
}
std::vector<Task> generate_nested_ipcp_pip_prop(float u_total) {
    return generate_nested(u_total, Subtask::fixed, Subtask::inherited, Subtask::propagated);
}
std::vector<Task> generate_nested_ipcp_prop_pip(float u_total) {
    return generate_nested(u_total, Subtask::fixed, Subtask::propagated, Subtask::inherited);
}

std::vector<Task> generate_nested(float u_total, Subtask::Type c_type, Subtask::Type d_type, Subtask::Type e_type) {

    Task t1, t2, t3, t4;


    Subtask t1_1{ "1_1" };
    Subtask A_1{ "A_1", Subtask::inherited };
    Subtask C_1{ "C_1", c_type};
    Subtask E{ "E", e_type};
    Subtask C_2{ "C_2", c_type};
    Subtask D_1{ "D_1", d_type};
    Subtask D_2{ "D_2", d_type};
    Subtask A_2{ "A_2", Subtask::inherited };
    Subtask t1_2{ "1_2" };

    Subtask t2_1{ "2_1" };
    Subtask t2_2{ "2_2" };

    Subtask t3_1{ "3_1" };
    Subtask B_1{ "B_1", Subtask::inherited };
    Subtask B_2{ "B_2", Subtask::inherited };
    Subtask t3_2{ "3_2" };

    Subtask t4_1{ "4_1" };
    Subtask t4_2{ "4_2" };

    //Describe task subtasks
    t1.subtasks = std::vector<Subtask>{ t1_1, A_1, C_1, E, C_2, D_1, E, D_2, A_2, t1_2 };
    t2.subtasks = std::vector<Subtask>{ t2_1, A_1, C_1, E, C_2, D_1, E, D_2, A_2, t2_2 };
    t3.subtasks = std::vector<Subtask>{ t3_1, B_1, C_1, E, C_2, D_1, E, D_2, B_2, t3_2 };
    t4.subtasks = std::vector<Subtask>{ t4_1, B_1, C_1, E, C_2, D_1, E, D_2, B_2, t4_2 };

    //Create vector of tasks
    std::vector<Task> tasks;
    tasks.push_back(t1); tasks.push_back(t2); tasks.push_back(t3); tasks.push_back(t4);

    std::vector<Task> tasks_generated = generate(u_total, tasks);

    return tasks_generated;
}