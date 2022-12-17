#include "task.h"
#include "../timing_headers/benchmarked_times.h"

Subtask::Subtask(const char * name_, Type type_) :
	C(0), name(name_), type(type_) {}

std::string Subtask::protocol(void) {
	switch (type) {
	case propagated:
		return "propagated";
	case fixed:
		return "fixed";
	case inherited:
		return "inherited";
	default:
		return "";
	}
}

void Task::init(int index_, float U_, float T_) {
	index = index_, U = U_; T = T_; C = U * T;
}

void Task::init(int index_, float U_, int period_index) {
	index = index_; U = U_;
	T = harmonic_periods_ms[period_index] * 1000; //Convert from ms to us
	C = U * T;
	P = (num_harmonic_periods - period_index) * 2;
	overhead = (float)dispatch_overhead/(float)cpu_speed;
}