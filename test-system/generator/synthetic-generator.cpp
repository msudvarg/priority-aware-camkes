#include "task.h"
#include "task-generators.h"
#include "prng.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <iostream>

//Assign utilizations according to uunisort for n tasks with total utilization U
std::vector<float> uunisort(const float U, const unsigned n) {

    std::uniform_real_distribution<float> d(0, U);
    std::set<float> boundaries;
    std::vector<float> utilizations;

    for (unsigned i = 0; i < n - 1; ++i) {
        boundaries.insert(d(PRNG::get()));
    }

    float low = 0;
    for (float f : boundaries) {
        utilizations.push_back(f - low);
        low = f;
    }
    
    utilizations.push_back(U - low);

    return utilizations;

}

//Select period at random from harmonic candidates
int gen_period() {
    std::uniform_int_distribution<int> d(0, num_harmonic_periods-1);
    return d(PRNG::get()); //Convert to us from ms
}

//Try to generate a taskset
std::vector<Task> try_generate(float u_total, std::vector<Task> tasks) {

    //Number of tasks
    const unsigned len = tasks.size();

    //Assign task utilizations according to UUniSort
    std::vector<float> utilizations = uunisort(u_total, len);
    for (unsigned i = 0; i < len; ++i) {
        tasks[i].init(i + 1, utilizations[i], gen_period());
    }

    //Sort tasks according to increasing workload
    std::sort(tasks.begin(), tasks.end(),
        [](const Task & a, const Task & b)->bool{
            return a.C < b.C;
        });

    //For each task, assign workloads to subtasks according to UUniSort
    //If a subtask is shared by another task, assign equal workloads
    for (unsigned i = 0; i < len; ++i) {
        Task & task = tasks[i];

        //Get subtasks that need workloads assigned, and the total workload required
        float C = task.C - task.overhead;
        std::set<std::string> subtasks;
        for (Subtask & subtask : task.subtasks) {

            //Subtask already has an assigned workload
            if (subtask.C != 0) {
                C -= subtask.C;
            }

            //Subtask available, insert into set to avoid duplicates
            else {
                subtasks.insert(subtask.name);
            }
        }
        if (C < 0) {
            return std::vector<Task> {};
        }

        //Assign workloads to subtasks
        std::vector<float> workloads = uunisort(C, subtasks.size());

        //Only assign to those not yet assigned
        unsigned workload_idx = 0;
        for (Subtask & subtask : task.subtasks) {
            if (subtask.C == 0) {

                std::string name = subtask.name;

                //Count the number of duplicate subtasks
                int dups = 0;
                for (Subtask & subtask : task.subtasks) {
                    if(subtask.name == name) ++dups;
                }

                //Get workload
                float C = workloads[workload_idx];
                if(dups > 1) C /= (float)dups;
                subtask.C = C;

                //Assign to other tasks sharing the subtask
                //Start with j = i so that duplicate subtasks in a task are assigned same workloads
                for (unsigned j = i; j < len; ++j) {
                    Task & task = tasks[j];
                    for (Subtask & subtask : task.subtasks) {
                        if (subtask.name == name) subtask.C = C;
                    }
                }

                workload_idx++;

            }
        }
    }

    return tasks;
}

//Verify the tasksest has been generated correctly
bool check_tasks(float u_total, std::vector<Task> & tasks) {
    std::sort(tasks.begin(), tasks.end(), Sort_Task_Priority{});
    float u = 0;
    for (auto it = tasks.begin(); it != tasks.end(); ++it) {

        //Guarantee correct priority order
        auto next = it + 1;
        if (next != tasks.cend()) {
            if (next->T > it->T) {
                std::cerr << "Tasks not in priority order" << std::endl;
                return false;
            }
        }  

        //Guarantee subtask execution adds up
        float c = 0;
        for (const Subtask subtask : it->subtasks) {
            c += subtask.C;
        }
        if (abs(c - it->C + it->overhead)/c > 0.000001) {
            std::cerr << "Subtask execution does not add to task execution" << std::endl;
            return false;
        }

        //Guarantee task utilization adds up
        u += it->U;

        //Guarantee workload and period make utilization
        if (abs(it->C/it->T - it->U)/it->U > 0.000001) {
            std::cerr << "Workload and period do not make utilization" << std::endl;
            return false;
        }
    }

    //Guarantee task utilization adds up
    if (abs(u - u_total)/u > 0.000001) {
        std::cerr << "Task utilization does not equal total" << std::endl;
        return false;
    }

    return true;
}

//Repeatedly try to generate a taskset until successful, then check the taskset
std::vector<Task> generate(float u_total, std::vector<Task> & tasks) {

    int num_bad = -1;
    std::vector<Task> tasks_generated;
    while (!tasks_generated.size()) {
        num_bad++;
        tasks_generated = try_generate(u_total, tasks);
    }

    if (!check_tasks(u_total, tasks_generated)) throw 1;

    return tasks_generated;

}