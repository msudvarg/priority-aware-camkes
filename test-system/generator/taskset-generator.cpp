
#include "prng.h"
#include "task.h"
#include "taskset_common.h"
#include "task-generators.h"

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>


void generate_camkes(
	float U,
	std::vector<Task> (*generate_func)(float),
	std::string component_name, int nthreads_c, int nthreads_d, int nthreads_e)
{

	std::string filename = "../task_systems/" + component_name + ".camkes";
	std::ofstream outfile(filename);

	std::vector<Task> tasks = generate_func(U);
	std::sort(tasks.begin(), tasks.end(), Sort_Task_Index{});

	Task & t1 = tasks[0];
	Subtask & t1_1 = t1.subtasks[0];
	Subtask & A_1 = t1.subtasks[1];
    Subtask & C_1 = t1.subtasks[2];
    Subtask & E = t1.subtasks[3];
    Subtask & C_2 = t1.subtasks[4];
    Subtask & D_1 = t1.subtasks[5];
    Subtask & D_2 = t1.subtasks[7];
    Subtask & A_2 = t1.subtasks[8];
    Subtask & t1_2 = t1.subtasks[9];

	Task & t2 = tasks[1];
    Subtask & t2_1 = t2.subtasks[0];
    Subtask & t2_2 = t2.subtasks[9];

	Task & t3 = tasks[2];
    Subtask & t3_1 = t3.subtasks[0];
    Subtask & B_1 = t3.subtasks[1];
    Subtask & B_2 = t3.subtasks[8];
    Subtask & t3_2 = t3.subtasks[9];

	Task & t4 = tasks[3];
    Subtask & t4_1 = t4.subtasks[0];
    Subtask & t4_2 = t4.subtasks[9];

	int hyperperiod = std::max({t1.T, t2.T, t3.T, t4.T});
	constexpr int hyperperiods = 10;
	constexpr unsigned d_p = num_harmonic_periods * 2;

    outfile << "#define l1a_num_threads 2\n";
    outfile << "#define l1b_num_threads 2\n";
    outfile << "#define l2_r1_num_threads " << nthreads_c << '\n';
    outfile << "#define l2_r2_num_threads " << nthreads_d << '\n';
    outfile << "#define l3_num_threads " << nthreads_e << '\n';

	outfile << test_system_header_0;

	outfile << "component " << component_name << " {\n";

	outfile << test_system_header_1;

	outfile << "t1._priority = " << t1.P << ";\n";
	outfile << "t1.priority = " << t1.P << ";\n";
	outfile << "t1.execution_time_pre = " << (int)t1_1.C << ";\n";
	outfile << "t1.execution_time_post = " << (int)t1_2.C << ";\n";
	outfile << "d1.priority = " << t1.P + d_p << ";\n";
	outfile << "d1.release_time = " << 0 << ";\n";
	outfile << "d1.period = " << (int)t1.T << ";\n";
	outfile << "d1.periods = " << hyperperiods * hyperperiod/t1.T << ";\n";

	outfile << "t2._priority = " << t2.P << ";\n";
	outfile << "t2.priority = " << t2.P << ";\n";
	outfile << "t2.execution_time_pre = " << (int)t2_1.C << ";\n";
	outfile << "t2.execution_time_post = " << (int)t2_2.C << ";\n";
	outfile << "d2.priority = " << t2.P + d_p << ";\n";
	outfile << "d2.release_time = " << 0 << ";\n";
	outfile << "d2.period = " << (int)t2.T << ";\n";
	outfile << "d2.periods = " << hyperperiods * hyperperiod / t2.T << ";\n";

	outfile << "t3._priority = " << t3.P << ";\n";
	outfile << "t3.priority = " << t3.P << ";\n";
	outfile << "t3.execution_time_pre = " << (int)t3_1.C << ";\n";
	outfile << "t3.execution_time_post = " << (int)t3_2.C << ";\n";
	outfile << "d3.priority = " << t3.P + d_p << ";\n";
	outfile << "d3.release_time = " << 0 << ";\n";
	outfile << "d3.period = " << (int)t3.T << ";\n";
	outfile << "d3.periods = " << hyperperiods * hyperperiod / t3.T << ";\n";

	outfile << "t4._priority = " << t4.P << ";\n";
	outfile << "t4.priority = " << t4.P << ";\n";
	outfile << "t4.execution_time_pre = " << (int)t4_1.C << ";\n";
	outfile << "t4.execution_time_post = " << (int)t4_2.C << ";\n";
	outfile << "d4.priority = " << t4.P + d_p << ";\n";
	outfile << "d4.release_time = " << 0 << ";\n";
	outfile << "d4.period = " << (int)t4.T << ";\n";
	outfile << "d4.periods = " << hyperperiods * hyperperiod / t4.T << ";\n";

    outfile << test_system_threads;

	unsigned int A_P = std::max(t1.P, t2.P) + 1;
	outfile << "l1a.r_priority = " << A_P << ";\n"; //Max of t1 and t2
	outfile << "l1a.r_priority_protocol = \"" << A_1.protocol() << "\";\n";
	outfile << "l1a.r_execution_time = " << (int)A_1.C << ";\n";
	outfile << "l1a.r_execution_time_post = " << (int)A_2.C << ";\n";

	unsigned int B_P = std::max(t3.P, t4.P) + 1;
	outfile << "l1b.r_priority = " << B_P << ";\n"; //Max of t3 and t4
	outfile << "l1b.r_priority_protocol = \"" << B_1.protocol() << "\";\n";
	outfile << "l1b.r_execution_time = " << (int)B_1.C << ";\n";
	outfile << "l1b.r_execution_time_post = " << (int)B_2.C << ";\n";

    unsigned int C_P = std::max(A_P, B_P);
	outfile << "l2.r1_priority = " << C_P << ";\n"; //Max of all tasks
	outfile << "l2.r1_priority_protocol = \"" << C_1.protocol() << "\";\n";
	outfile << "l2.r1_execution_time = " << (int)C_1.C << ";\n";
	outfile << "l2.r1_execution_time_post = " << (int)C_2.C << ";\n";

    unsigned int D_P = std::max(A_P, B_P);
	outfile << "l2.r2_priority = " << D_P << ";\n"; //Max of all tasks
	outfile << "l2.r2_priority_protocol = \"" << D_1.protocol() << "\";\n";
	outfile << "l2.r2_execution_time = " << (int)D_1.C << ";\n";
	outfile << "l2.r2_execution_time_post = " << (int)D_2.C << ";\n";

    unsigned int E_P = std::max(A_P, B_P);
	outfile << "l3.r_priority = " << E_P << ";\n"; //Max of all tasks
	outfile << "l3.r_priority_protocol = \"" << E.protocol() << "\";\n";
	outfile << "l3.r_execution_time = " << (int)E.C << ";\n";

    outfile << test_system_footer << std::endl;


}


int main(int argc, char * argv[]) {


	PRNG::seed(0);

	//For CAmkES Hardware Tests 
	for (int i = 1; i <= 15; ++i) {
		for (int j = 0; j < 10; ++j) {
			std::string u = std::to_string(i) + "_" + std::to_string(j);
			generate_camkes((float)i/100., generate_nested_pip_prop_pip, "pip_prop_pip_" + u, 3,3,3); //2,2,3
			generate_camkes((float)i/100., generate_nested_pip_prop_prop, "pip_prop_prop_" + u, 3,3,3); //2,2,3
			generate_camkes((float)i/100., generate_nested_ipcp_pip_prop, "ipcp_pip_prop_" + u, 1,3,3); //1,2,2
			generate_camkes((float)i/100., generate_nested_ipcp_prop_pip, "ipcp_prop_pip_" + u, 1,3,3); //1,2,2
		}
	}

	return 0;
}