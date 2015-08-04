#pragma once

#include "stdafx.hpp"
#include "SD_Task_Dependency.hpp"
#include "SD_Workstation.hpp"

class SD_Workstation;
class SD_Task_Dependency;

class SD_Task
{
private:
	friend class SD_Simulation;								/** Give access to private member inside SD_Simulation member functions */
public:
	enum STATE
	{
		NOT_SCHEDULED = 0,      							/**< @brief Initial state (not valid for SD_watch and SD_unwatch) */
		SCHEDULABLE = 0x0001,   							/**< @brief A task becomes SD_SCHEDULABLE as soon as its dependencies are satisfied */
		SCHEDULED = 0x0002,     							/**< @brief A task becomes SD_SCHEDULED when you call function SD_task_schedule. SD_simulate will execute it when it becomes SD_RUNNABLE */
		RUNNABLE = 0x0004,      							/**< @brief A scheduled task becomes runnable is SD_simulate as soon as its dependencies are satisfied */
		IN_FIFO = 0x0008,       							/**< @brief A runnable task can have to wait in a workstation FIFO if the workstation is sequential */
		RUNNING = 0x0010,       							/**< @brief An SD_RUNNABLE or SD_IN_FIFO becomes SD_RUNNING when it is launched */
		DONE = 0x0020,          							/**< @brief The task is successfully finished */
		FAILED = 0x0040         							/**< @brief A problem occurred during the execution of the task */
	};

	enum KIND
	{
		NOT_TYPED = 0,      						/**< @brief no specified type */
		COMM_E2E = 1,       						/**< @brief end to end communication */
		COMP_SEQ = 2,       						/**< @brief sequential computation */
		COMP_PAR_AMDAHL = 3, 						/**< @brief parallel computation (Amdahl's law) */
		COMM_PAR_MXN_1D_BLOCK = 4 					/**< @brief MxN data redistribution (1D Block distribution) */
	};

	/** Creates a new task */
	static SD_Task* create(const char* name, void* data, double amount);
	/** Create a sequential computation task that can then be auto-scheduled */
	static SD_Task* create_comp_seq(const char* name, void *data, double amount);
	/** Create a parallel computation task that can then be auto-scheduled */
	static SD_Task* create_comp_par_amdahl(const char* name, void *data, double amount, double alpha);
	/** Create a end-to-end communication task that can then be auto-scheduled */
	static SD_Task* create_comm_e2e(const char* name, void *data, double amount);
	/** Create a complex data redistribution task that can then be auto-scheduled */
	static SD_Task* create_comm_par_mxn_1d_block(const char* name, void *data, double amount);

	/** Returns the state of the task */
	void set_state(STATE state);
	/** Returns the name of this task */
	inline char* get_name() {return name;}
	/** Allows to change the name of this task */
	void set_name(char* name);

	/** Returns the user data of this task */
	void* get_data();
	/** Sets the user data of this task */
	void set_data(void* data);

	/** Sets the rate of this task */
	void set_rate(double rate);
	/** Adds a watch point to this task */
	void watch(STATE state);
	/** Removes a watch point from this task */
	void unwatch(STATE state);

	/** Returns the total amount of work contained in this task */
	double get_amount();
	/** Returns the remaining amount work to do till the completion of this task */
	double get_remaining_amount();

	/** Sets the total amount of work of this task. For sequential typed tasks (COMP_SEQ and COMM_E2E),
	 *  it also sets the appropriate values in the computation_amount and communication_amount arrays respectively.
	 *  Nothing more than modifying task->amount is done for paralle typed tasks (COMP_PAR_AMDAHL and COMM_PAR_MXN_1D_BLOCK)
	 *  as the distribution of the amount of work is done at scheduling time */
	void set_amount(double amount);

	/** Returns the alpha parameter of this SD_TASK_COMP_PAR_AMDAHL task */
	double get_alpha();
	/** Returns an approximative estimation of the execution time of this task */
	double get_execution_time(const double* computation_amount, const double* communication_amount);

	/** Schedules a task */
	void schedule(const std::vector<SD_Workstation*>* workstation_list, const double *computation_amount, const double *communication_amount, double rate);
	/** Unschedules a task */
	void unschedule();

	/** Returns the start time of this task */
	double get_start_time();
	/** Returns the finish time of this task */
	double get_finish_time();

	/** Returns the vector of the parents of this task */
	const std::vector<SD_Task_Dependency*>* get_parent();
	/** Returns the vector of the parents of this task */
	const std::vector<SD_Task_Dependency*>* get_children();

	/** Returns the list of workstations involved in a task */
	const std::vector<SD_Workstation*>* get_workstation_list();

	/** Destroy this task */
	void destroy();
	/** Displays debugging informations about a task */
	void dump();
	/** Dumps the task in dotty formalism into the FILE* passed as second argument */
	void dotty(FILE* out_FILE);
	/** Blah */
	void distribute_comp_amdahl(int ws_count);

	/** Autoschedule a task on a list of workstations */
	void schedulel(int count, ...);
	/** Auto-schedules a task */
	void schedulev(int count);

	inline STATE get_state() {return state;}

	bool is_scheduled();
	bool is_schedulable();
	bool is_runnable();
	bool is_running();
	bool is_in_fifo();

	inline void set_unsatisfied_dependencies(int u_d) {unsatisfied_dependencies = u_d;}
	inline void set_is_not_ready(unsigned int u_i) {is_not_ready = u_i;}

	bool try_to_run();
	void just_done();
	void really_run();

	inline KIND get_kind() {return kind;}

private:
	static SD_Task* create_sized(const char *name, void *data, double amount, int ws_count);
	void do_schedule();
	void destroy_scheduling_data();

	SD_Swag<SD_Task*>* state_set = nullptr;

	//dependencies
	std::vector<SD_Task_Dependency*> tasks_before;
	std::vector<SD_Task_Dependency*> tasks_after;
	int unsatisfied_dependencies;
	unsigned int is_not_ready;

	STATE state;
	KIND kind;
	void *data;                  							//user data
	char *name;
	double amount;
	double alpha;        									//used by typed parallel tasks
	double remains;
	double start_time;
	double finish_time;
	surf_action_t surf_action;
	unsigned short watch_points; 							//bit field xor()ed with masks

	bool fifo_checked;            							//used by SD_task_just_done to make sure we evaluate the task only once
	bool marked;                 							//used to check if the task DAG has some cycle

	//scheduling parameters (only exist in state SD_SCHEDULED)
	std::vector<SD_Workstation*> workstation_list;   		//surf workstation
	double* flops_amount;
	double* bytes_amount;
	double rate;

	long long int counter;       							//task unique identifier for instrumentation
	char *category;											//sd task category for instrumentation
};
