#pragma once

#include "task.h"

std::vector<Task> generate_nested_pip_prop_pip(float u_total);
std::vector<Task> generate_nested_pip_prop_prop(float u_total);
std::vector<Task> generate_nested_ipcp_pip_prop(float u_total);
std::vector<Task> generate_nested_ipcp_prop_pip(float u_total);

std::vector<Task> generate_nested(float u_total, Subtask::Type c_type, Subtask::Type d_type, Subtask::Type e_type);

std::vector<Task> generate(float u_total, std::vector<Task> & tasks);