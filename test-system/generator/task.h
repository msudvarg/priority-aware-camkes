#pragma once

#include <vector>
#include <string>

// constexpr unsigned num_harmonic_periods = 6;
// const static float harmonic_periods_ms[] = { 5, 10, 50, 100, 500, 1000 };

constexpr unsigned num_harmonic_periods = 5;
const static float harmonic_periods_ms[] = { 10, 50, 100, 500, 1000 };

struct Subtask {

	enum Type {
		task,
		propagated,
		inherited,
		fixed
	};

	float C;
	std::string name;
	Type type;

	Subtask(const char * name_, Type type_ = task);
	std::string protocol(void);
};

struct Task {
	float U = 0;
	float T = 0;
	float C = 0;
	float B = 0;
	unsigned P = 0;
	int index = 0;
	std::vector<Subtask> subtasks;

	void init(int index_, float U_, float T_);
	void init(int index_, float U_, int period_index);


};

struct Sort_Task_Index {
	bool operator()(const Task & a, const Task & b) const {
		return a.index < b.index;
	}
};

struct Sort_Task_Priority {
	bool operator()(const Task & a, const Task & b) const {
		return a.P < b.P;
	}
};