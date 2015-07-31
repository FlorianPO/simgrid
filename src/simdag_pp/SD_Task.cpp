#include "SD_Task.h"

#include "../simdag/private.h"
#include "SD_Route.h"
#include "SD_Simulation.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(sd_task, sd, "Logging specific to SimDag (task)");

SD_Task* SD_Task::create(const char* name, void* data, double amount)
{
	SD_Task* task = new SD_Task();

	task->data = data;
	task->name = xbt_strdup(name);
	task->amount = amount;
	task->remains = amount;

	task_number++;

	//TRACE_sd_task_create(task);

	return task;
}

SD_Task* SD_Task::create_sized(const char *name, void *data, double amount, int ws_count)
{
	SD_Task* task = create(name, data, amount);
	task->bytes_amount = xbt_new0(double, ws_count * ws_count);
	task->flops_amount = xbt_new0(double, ws_count);
	task->workstation_list.resize(ws_count);

	return task;
}

SD_Task* SD_Task::create_comp_seq(const char *name, void *data, double amount)
{
	SD_Task* res = create_sized(name, data, amount, 1);

	res->flops_amount[0] = amount;
	res->kind = COMP_SEQ;

	//TRACE_category("COMP_SEQ");
	//TRACE_sd_set_task_category(res, "COMP_SEQ");

	return res;
}

SD_Task* SD_Task::create_comp_par_amdahl(const char* name, void *data, double amount, double alpha)
{
	xbt_assert(alpha < 1. && alpha >= 0., "Invalid parameter: alpha must be in [0.;1.[");

	SD_Task* res = create(name, data, amount);
	res->alpha = alpha;
	res->kind = COMP_PAR_AMDAHL;

	//TRACE_category("COMP_PAR_AMDAHL");
	//TRACE_sd_set_task_category(res, "COMP_PAR_AMDAHL");

	return res;
}

SD_Task* SD_Task::create_comm_e2e(const char* name, void *data, double amount)
{
	SD_Task* res = create_sized(name, data, amount, 2);
	res->bytes_amount[2] = amount;
	res->kind = COMM_E2E;

	//TRACE_category("COMM_E2E");
	//TRACE_sd_set_task_category(res, "COMM_E2E");

	return res;
}

SD_Task* SD_Task::create_comm_par_mxn_1d_block(const char* name, void *data, double amount)
{
	SD_Task* res = create(name, data, amount);
	res->workstation_list.clear();
	res->kind = COMM_PAR_MXN_1D_BLOCK;

	//TRACE_category("COMM_PAR_MXN_1D_BLOCK");
	//TRACE_sd_set_task_category(res, "COMM_PAR_MXN_1D_BLOCK");

	return res;
}

void SD_Task::set_name(char* name)
{
	xbt_free(this->name);
	name = xbt_strdup(name);
}

void* SD_Task::get_data() {return data;}

void SD_Task::set_rate(double rate)
{
	xbt_assert(kind == COMM_E2E, "The rate can be modified for end-to-end communications only.");
	if (start_time < 0)
		this->rate = rate;
	else
		XBT_WARN("Task %p has started. Changing rate is ineffective.", this);
}

void SD_Task::watch(STATE state)
{
	if (state & SCHEDULED)
		THROWF(arg_error, 0, "Cannot add a watch point for state SD_NOT_SCHEDULED");

	watch_points = watch_points | state;
	//__SD_print_watch_points(task);
}

void SD_Task::unwatch(STATE state)
{
	 xbt_assert(state != NOT_SCHEDULED, "SimDag error: Cannot have a watch point for state SD_NOT_SCHEDULED");
	 watch_points = watch_points & ~state;
	 //__SD_print_watch_points(task);
}

double SD_Task::get_amount() {return amount;}

double SD_Task::get_remaining_amount()
{
	if (surf_action)
		return surf_action_get_remains(surf_action);
	else
		return remains;
}

void SD_Task::set_amount(double amount)
{
	this->amount = amount;
	if (kind == COMP_SEQ)
		flops_amount[0] = amount;
	else if (kind == COMM_E2E)
		bytes_amount[2] = amount;
}

double SD_Task::get_alpha()
{
	xbt_assert(kind == COMP_PAR_AMDAHL, "Alpha parameter is not defined for this kink of task");
	return alpha;
}

double SD_Task::get_execution_time(const double* computation_amount, const double* communication_amount)
{
	double time = 0.0;
	double max_time = 0.0;

	//the task execution time is the maximum execution time of the parallel tasks
	for (unsigned int i = 0; i < workstation_list.size(); i++)
	{
		time = 0.0;
		if (flops_amount != nullptr)
			time = workstation_list[i]->get_computation_time(flops_amount[i]);

		if (bytes_amount != nullptr)
			for (unsigned int j = 0; j < workstation_list.size(); j++)
				time += SD_Route::get_communication_time(workstation_list[i], workstation_list[j], bytes_amount[i * workstation_list.size() + j]);
	}

	if (time > max_time)
		max_time = time;

	return max_time;
}

void SD_Task::schedule(const std::vector<SD_Workstation*>* workstation_list, const double *computation_amount, const double *communication_amount, double rate)
{
	int communication_nb;
	this->workstation_list.clear();
	this->rate = -1;

	this->workstation_list.resize(workstation_list->size());
	this->rate = rate;

	if (flops_amount)
	{
		flops_amount = static_cast<double*>(xbt_realloc(flops_amount, sizeof(double) * workstation_list->size()));
		memcpy(flops_amount, flops_amount, sizeof(double) * workstation_list->size());
	}
	else
	{
		xbt_free(flops_amount);
		flops_amount = nullptr;
	}

	communication_nb = workstation_list->size() * workstation_list->size();
	if (bytes_amount)
	{
		bytes_amount = static_cast<double*>(xbt_realloc(bytes_amount, sizeof(double) * communication_nb));
		memcpy(bytes_amount, bytes_amount, sizeof(double) * communication_nb);
	}
	else
	{
		xbt_free(bytes_amount);
		bytes_amount = nullptr;
	}

	this->workstation_list = *workstation_list;

	do_schedule();
}

void SD_Task::unschedule()
{
	if (state_set != &SD_Simulation::_t->scheduled_task_set && state_set != &SD_Simulation::_t->runnable_task_set &&
		state_set != &SD_Simulation::_t->running_task_set && state_set != &SD_Simulation::_t->failed_task_set)
		  THROWF(arg_error, 0, "Task %s: the state must be SD_SCHEDULED, SD_RUNNABLE, SD_RUNNING or SD_FAILED", name);


	if ((is_scheduled() || is_runnable()) && (kind == COMP_PAR_AMDAHL || kind == COMM_PAR_MXN_1D_BLOCK))
	{
		/* Don't free scheduling data for typed tasks */
		destroy_scheduling_data();
		workstation_list.clear();
	}

	if (is_running())       /* the task should become SD_FAILED */
		surf_action_cancel(surf_action);
	else
	{
		if (unsatisfied_dependencies == 0)
			set_state(SCHEDULABLE);
		else
			set_state(NOT_SCHEDULED);
	}
	remains = amount;
	start_time = -1.0;
}

void SD_Task::do_schedule()
{
	if (is_scheduled() || is_schedulable())
	    THROWF(arg_error, 0, "Task '%s' has already been scheduled", name);

	//update the task state
	if (unsatisfied_dependencies == 0)
		state = RUNNABLE;
	else
		state = SCHEDULED;
}

void SD_Task::destroy_scheduling_data()
{
	if (!(is_scheduled() || is_runnable()) && !is_in_fifo())
		THROWF(arg_error, 0, "Task '%s' must be SD_SCHEDULED, SD_RUNNABLE or SD_IN_FIFO", name);

	xbt_free(flops_amount);
	xbt_free(bytes_amount);
	flops_amount = bytes_amount = nullptr;
}

void SD_Task::set_state(STATE state)
{
	if (state_set != nullptr)
		state_set->remove(this);
	switch (state)
	{
		case NOT_SCHEDULED:
			state_set = &SD_Simulation::_t->not_scheduled_task_set; break;
		case SCHEDULABLE:
			state_set = &SD_Simulation::_t->schedulable_task_set; break;
		case SCHEDULED:
			state_set = &SD_Simulation::_t->scheduled_task_set; break;
		case RUNNABLE:
			state_set = &SD_Simulation::_t->runnable_task_set; break;
		case IN_FIFO:
			state_set = &SD_Simulation::_t->in_fifo_task_set; break;
		case RUNNING:
			state_set = &SD_Simulation::_t->running_task_set;
			start_time = surf_action_get_start_time(surf_action);
			break;
		case DONE:
			state_set = &SD_Simulation::_t->done_task_set;
			finish_time = surf_action_get_finish_time(surf_action);
			remains = 0;
			#ifdef HAVE_JEDULE
				jedule_log_sd_event(task);
			#endif
			break;
		case FAILED:
			state_set = &SD_Simulation::_t->failed_task_set; break;
		default:
			xbt_die("Invalid state");
	}
	state_set->push_back(this);

	this->state = state;
	if (watch_points & state)
	{
		XBT_VERB("Watch point reached with task '%s'!", name);
		sd_global->watch_point_reached = 1;
		unwatch(state); //remove the watch point
	}
}
double SD_Task::get_start_time()
{
	if (surf_action)
		return surf_action_get_start_time(surf_action);
	else
		return start_time;
}
double SD_Task::get_finish_time()
{

	if (surf_action)        /* should never happen as actions are destroyed right after their completion */
		return surf_action_get_finish_time(surf_action);
	else
		return finish_time;
}

const std::vector<SD_Task_Dependency*>* SD_Task::get_parent() {return &tasks_before;}

const std::vector<SD_Task_Dependency*>* SD_Task::get_children() {return &tasks_after;}

const std::vector<SD_Workstation*>* SD_Task::get_workstation_list() {return &workstation_list;}

void SD_Task::destroy()
{
	XBT_DEBUG("Destroying task %s...", name);

	for (unsigned int i=0; i < tasks_before.size(); i++)
		tasks_before[i]->remove(this);
	for (unsigned int i=0; i < tasks_after.size(); i++)
		tasks_after[i]->remove(this);

	destroy_scheduling_data();
	if (state_set != nullptr) /* would be null if just created */
		state_set->remove(this);

	xbt_free(name);
	if (surf_action != nullptr)
		surf_action_unref(surf_action);

	xbt_free(bytes_amount);
	xbt_free(flops_amount);

	//TRACE_sd_task_destroy(task);

	SD_Simulation::_t->task_number--;

	XBT_DEBUG("Task destroyed.");
}

void SD_Task::dump()
{
	char *statename;

	XBT_INFO("Displaying task %s", name);
	statename = bprintf("%s %s %s %s %s %s %s %s",
				  (state == NOT_SCHEDULED ? "not scheduled" : ""),
				  (state == SCHEDULABLE ? "schedulable" : ""),
				  (state == SCHEDULED ? "scheduled" : ""),
				  (state == RUNNABLE ? "runnable" : "not runnable"),
				  (state == IN_FIFO ? "in fifo" : ""),
				  (state == RUNNING ? "running" : ""),
				  (state == DONE ? "done" : ""),
				  (state == FAILED ? "failed" : ""));
	XBT_INFO("  - state: %s", statename);
	free(statename);

	if (kind != 0)
		switch (kind)
		{
			case COMM_E2E:
				XBT_INFO("  - kind: end-to-end communication"); break;
			case COMP_SEQ:
				XBT_INFO("  - kind: sequential computation"); break;
			case COMP_PAR_AMDAHL:
				XBT_INFO("  - kind: parallel computation following Amdahl's law"); break;
			case COMM_PAR_MXN_1D_BLOCK:
				XBT_INFO("  - kind: MxN data redistribution assuming 1D block distribution"); break;
			default:
				XBT_INFO("  - (unknown kind %d)", kind);
		}

	if (category)
		XBT_INFO("  - tracing category: %s", category);

	XBT_INFO("  - amount: %.0f", get_amount());
	if (kind == COMP_PAR_AMDAHL)
		XBT_INFO("  - alpha: %.2f", alpha);
	XBT_INFO("  - Dependencies to satisfy: %d", unsatisfied_dependencies);
	if (tasks_before.size() > 0)
	{
		XBT_INFO("  - pre-dependencies:");
		for (unsigned int i=0; i < tasks_before.size(); i++)
			XBT_INFO("    %s", tasks_before[i]->getDst());

	}
	if (tasks_after.size() > 0)
	{
		XBT_INFO("  - post-dependencies:");
		for (unsigned int i=0; i < tasks_after.size(); i++)
			XBT_INFO("    %s", tasks_after[i]->getDst());
	}
}

void SD_Task::dotty(FILE* out_FILE)
{
	fprintf(out_FILE, "  T%p [label=\"%.20s\"", this, name);
	switch (kind)
	{
		case COMM_E2E:
		case COMM_PAR_MXN_1D_BLOCK:
			fprintf(out_FILE, ", shape=box");
			break;
		case COMP_SEQ:
		case COMP_PAR_AMDAHL:
			fprintf(out_FILE, ", shape=circle");
			break;
		default:
			xbt_die("Unknown task type!");
	}
	fprintf(out_FILE, "];\n");
	for (unsigned int i=0; i < tasks_before.size(); i++)
		fprintf(out_FILE, " T%p -> T%p;\n", tasks_before[i]->getSrc(), tasks_before[i]->getDst());
}

void SD_Task::distribute_comp_amdahl(int ws_count)
{
	xbt_assert(kind == COMP_PAR_AMDAHL, "Task %s is not a SD_TASK_COMP_PAR_AMDAHL typed task. Cannot use this function.", name);
	flops_amount = xbt_new0(double, ws_count);
	bytes_amount = xbt_new0(double, ws_count * ws_count);
	workstation_list.clear();
	workstation_list.resize(ws_count);

	for (int i=0;i < ws_count; i++)
		flops_amount[i] = (alpha + (1 - alpha)/ws_count) * amount;
}

void SD_Task::schedulel(int count, ...)
{
	workstation_list.clear();
	workstation_list.resize(count);

	schedulev(count);
}
void SD_Task::schedulev(int count)
{
	xbt_assert(kind != 0, "Task %s is not typed. Cannot automatically schedule it.", name);
	switch (kind)
	{
		case COMP_PAR_AMDAHL:
			distribute_comp_amdahl(count);
		case COMM_E2E:
		case COMP_SEQ:
			xbt_assert(int(this->workstation_list.size()) == count, "Got %d locations, but were expecting %d locations", count, this->workstation_list.size());
			for (int i = 0; i < count; i++)
				this->workstation_list[i] = list[i];
			if (kind == COMP_SEQ && !flops_amount)
			{
				/*This task has failed and is rescheduled. Reset the flops_amount*/
				flops_amount = xbt_new0(double, 1);
				flops_amount[0] = remains;
			}
			do_schedule();
			break;
		default:
			xbt_die("Kind of task %s not supported by SD_task_schedulev()", name);
	}
	if (kind == COMM_E2E)
	{
		SD_Workstation* ws_0 = this->workstation_list[0];
		SD_Workstation* ws_1 = this->workstation_list[1];
		XBT_VERB("Schedule comm task %s between %s -> %s. It costs %.f bytes", name, ws_0->get_name(), ws_1->get_name(), bytes_amount[2]);
	}

	/* Iterate over all children and parents being COMM_E2E to say where I am
	* located (and start them if runnable) */
	if (kind == COMP_SEQ)
	{
		SD_Workstation* ws_0 = this->workstation_list[0];
		SD_Workstation* ws_1;
		XBT_VERB("Schedule computation task %s on %s. It costs %.f flops", name, ws_0->get_name(), flops_amount[0]);

		for (unsigned int i=0; i < tasks_before.size(); i++)
		{
			SD_Task* before = tasks_before[i]->getSrc();

			if (before->kind == COMM_E2E)
			{
				before->workstation_list[1] = this->workstation_list[0];

				if (before->workstation_list[0] && (before->is_schedulable() || !before->is_scheduled()))
				{
					before->do_schedule();
					ws_0 = before->workstation_list[0];
					ws_1 = before->workstation_list[1];
					XBT_VERB ("Auto-Schedule comm task %s between %s -> %s. It costs %.f bytes", before->name, ws_0->get_name(), ws_1->get_name(), before->bytes_amount[2]);
				}
			}
		}

		for (unsigned int i=0; i < tasks_after.size(); i++)
		{
			SD_Task* after = tasks_after[i]->getSrc();
			if (after->kind == COMM_E2E)
			{
				after->workstation_list[0] = this->workstation_list[0];
				if (after->workstation_list[1] && (!after->is_scheduled() || after->is_schedulable()))
				{
					after->do_schedule();
					ws_0 = after->workstation_list[0];
					ws_1 = after->workstation_list[1];
					XBT_VERB ("Auto-Schedule comm task %s between %s -> %s. It costs %.f bytes", after->name, ws_0->get_name(), ws_1->get_name(), after->bytes_amount[2]);

				}
			}
		}
	}
	/* Iterate over all children and parents being MXN_1D_BLOCK to say where I am
	* located (and start them if runnable) */
	if (kind == COMP_PAR_AMDAHL)
	{
		XBT_VERB("Schedule computation task %s on %d workstations. %.f flops will be distributed following Amdahl's Law", name, this->workstation_list.size(), flops_amount[0]);
		for (unsigned int i=0; i < tasks_before.size(); i++)
		{
			SD_Task* before = tasks_before[i]->getSrc();

			if (before->kind == COMM_PAR_MXN_1D_BLOCK)
			{
				if (before->workstation_list.size() == 0)
				{
					XBT_VERB("Sender side of Task %s is not scheduled yet", before->name);
					before->workstation_list.clear();
					before->workstation_list.reserve(count);

					XBT_VERB("Fill the workstation list with list of Task '%s'", name);
					for (int j = 0; j < count; j++)
						before->workstation_list[j] = this->workstation_list[j];
				}
				else
				{
					XBT_VERB("Build communication matrix for task '%s'", before->name);
					int src_nb, dst_nb;
					double src_start, src_end, dst_start, dst_end;
					src_nb = before->workstation_list.size();
					dst_nb = count;
					before->workstation_list.resize(src_nb+count);
					for (int i=0; i<count; i++)
						before->workstation_list[src_nb+i] = this->workstation_list[i];

					xbt_free(before->flops_amount);
					xbt_free(before->bytes_amount);
					before->flops_amount = xbt_new0(double, before->workstation_list.size());
					before->bytes_amount = xbt_new0(double, before->workstation_list.size() * before->workstation_list.size());

					for (int i=0;i < src_nb;i++)
					{
						SD_Workstation* ws_0;
						SD_Workstation* ws_1;
						src_start = i*before->amount/src_nb;
						src_end = src_start + before->amount/src_nb;
						for (int j=0; j < dst_nb; j++)
						{
							dst_start = j*before->amount/dst_nb;
							dst_end = dst_start + before->amount/dst_nb;
							ws_0 = before->workstation_list[i];
							ws_1 = before->workstation_list[src_nb+j];
							XBT_VERB("(%s->%s): (%.2f, %.2f)-> (%.2f, %.2f)", ws_0->get_name(), ws_1->get_name(), src_start, src_end, dst_start, dst_end);
							if (src_end <= dst_start || dst_end <= src_start)
								before->bytes_amount[i*(src_nb+dst_nb)+src_nb+j]=0.0;
							else
								before->bytes_amount[i*(src_nb+dst_nb)+src_nb+j] = MIN(src_end, dst_end) - MAX(src_start, dst_start);
							XBT_VERB("==> %.2f", before->bytes_amount[i*(src_nb+dst_nb)+src_nb+j]);
						}
					}
					if (before->is_schedulable() || !before->is_scheduled())
					{
						before->do_schedule();
						XBT_VERB("Auto-Schedule redistribution task %s. Send %.f bytes from %d hosts to %d hosts.", before->name, before->amount, src_nb, dst_nb);
					}
				}
			}
		}

		for (unsigned int i=0; i < tasks_after.size(); i++)
		{
			SD_Task* after = tasks_after[i]->getSrc();
			if (after->kind == COMM_PAR_MXN_1D_BLOCK)
			{
				if (after->workstation_list.size() == 0)
				{
					XBT_VERB("Receiver side of Task '%s' is not scheduled yet", after->name);
					after->workstation_list.clear();
					after->workstation_list.resize(count);

					XBT_VERB("Fill the workstation list with list of Task '%s'", name);
					for (int j=0; j < count; j++)
						after->workstation_list[j] = this->workstation_list[j];
				}
				else
				{
					int src_nb, dst_nb;
					double src_start, src_end, dst_start, dst_end;
					src_nb = count;
					dst_nb = after->workstation_list.size();
					after->workstation_list.resize(dst_nb+count);

					for (int i=dst_nb - 1; i>=0; i--)
						after->workstation_list[count+i] = after->workstation_list[i];
					for (int i=0; i<count; i++)
						after->workstation_list[i] = this->workstation_list[i];

					xbt_free(after->flops_amount);
					xbt_free(after->bytes_amount);

					after->flops_amount = xbt_new0(double, after->workstation_list.size());
					after->bytes_amount = xbt_new0(double, after->workstation_list.size() * after->workstation_list.size());

					for (int i=0;i < src_nb;i++)
					{
						src_start = i*after->amount/src_nb;
						src_end = src_start + after->amount/src_nb;
						for (int j=0; j<dst_nb; j++)
						{
							dst_start = j*after->amount/dst_nb;
							dst_end = dst_start + after->amount/dst_nb;
							XBT_VERB("(%d->%d): (%.2f, %.2f)-> (%.2f, %.2f)", i, j, src_start, src_end, dst_start, dst_end);
							if (src_end <= dst_start || dst_end <= src_start)
								after->bytes_amount[i*(src_nb+dst_nb)+src_nb+j]=0.0;
							else
							{
								after->bytes_amount[i*(src_nb+dst_nb)+src_nb+j] =
								MIN(src_end, dst_end)- MAX(src_start, dst_start);
							}
							XBT_VERB("==> %.2f", after->bytes_amount[i*(src_nb+dst_nb)+src_nb+j]);
						}
					}

					if (after->is_schedulable() || !after->is_scheduled())
					{
						after->do_schedule();
						XBT_VERB ("Auto-Schedule redistribution task %s. Send %.f bytes from %d hosts to %d hosts.", after->name,after->amount, src_nb, dst_nb);
					}
				}
			}
		}
	}
}

bool SD_Task::try_to_run()
{
	bool can_start = true;

	xbt_assert(is_runnable(), "Task '%s' is not runnable! Task state: %d", name, get_state());

	for (unsigned int i = 0; i < workstation_list.size(); i++)
		can_start = can_start && !workstation_list[i]->is_busy();


	XBT_DEBUG("Task '%s' can start: %d", name, can_start);
	if (!can_start) /* if the task cannot start and is not in the FIFOs yet */
	{
		for (unsigned int i = 0; i < workstation_list.size(); i++)
			if (workstation_list[i]->get_access_mode() == SD_Workstation::SEQUENTIAL_ACCESS)
			{
				XBT_DEBUG("Pushing task '%s' in the FIFO of workstation '%s'", name, workstation_list[i]->get_name());
				workstation_list[i]->push_fifo(this);
			}

		set_state(IN_FIFO);
		xbt_assert(is_in_fifo(), "Bad state of task '%s': %d", name, get_state());
		XBT_DEBUG("Task '%s' state is now SD_IN_FIFO", name);
	}
	else
		really_run();

	return can_start;
}

void SD_Task::just_done()
{
	SD_Workstation* tmp;

	SD_Task* candidate;
	std::vector<SD_Task*> candidate_list;
	bool can_start = true;

	xbt_assert(is_running(), "The task must be running! Task state: %d", get_state());

	set_state(DONE);
	surf_action_unref(surf_action);
	surf_action = nullptr;

	XBT_DEBUG("Looking for candidates");

	/* if the task was executed on sequential workstations,maybe we can execute the next task of the FIFO for each workstation */
	for (unsigned int i = 0; i < workstation_list.size(); i++)
	{
		tmp = workstation_list[i];
		XBT_DEBUG("Workstation '%s': access_mode = %d", tmp->get_name(), tmp->get_access_mode());
		if (tmp->get_access_mode() == SD_Workstation::SEQUENTIAL_ACCESS)
		{
			xbt_assert(tmp->get_current_task() == this, "Workstation '%s': current task should be '%s'",tmp->get_name(), name);
			tmp->set_current_task(nullptr); /* the task is over so we can release the workstation */

			XBT_DEBUG("Getting candidate in FIFO");
			candidate = tmp->pop_fifo();

			if (candidate != nullptr)
			{
				XBT_DEBUG("Candidate: '%s'", candidate->get_name());
				xbt_assert(candidate->is_in_fifo(), "Bad state of candidate '%s': %d", candidate->get_name(), candidate->get_state());
			}

			XBT_DEBUG("Candidate in fifo: %p", candidate);

			if (candidate != NULL) 	/* if there was a task waiting for my place */
			{
				/* Unfortunately, we are not sure yet that we can execute the task now,
				because the task can be waiting more deeply in some other
				workstation's FIFOs ...
				So we memorize all candidate tasks, and then we will check for each
				candidate whether or not all its workstations are available. */

				/* register the candidate */
				candidate_list[candidate_list.size()] = candidate;
				candidate->fifo_checked = 0;
			}
		}
	}

	XBT_DEBUG("Candidates found: %d", int(candidate_list.size()));

	/* now we check every candidate task */
	for (unsigned int i = 0; i < candidate_list.size(); i++)
	{
		candidate = candidate_list[i];

		if (candidate->fifo_checked)
			continue;                 /* we have already evaluated that task */

		xbt_assert(candidate->is_in_fifo(), "Bad state of candidate '%s': %d", candidate->get_name(), candidate->get_state());
		for (unsigned int j = 0; j < candidate->workstation_list.size() && can_start; j++)
		{
			tmp = candidate->workstation_list[i];

			/* I can start on this workstation if the workstation is shared or if I am the first task in the FIFO */
			can_start = tmp->get_access_mode() == SD_Workstation::SHARED_ACCESS || candidate == tmp->front_fifo();
		}

		XBT_DEBUG("Candidate '%s' can start: %d", candidate->get_name(), can_start);

		/* now we are sure that I can start! */
		if (can_start)
		{
			for (unsigned int j = 0; j < candidate->workstation_list.size() && can_start; j++)  /* for each workstation */
			{
				tmp = candidate->workstation_list[i];
				/* update the FIFO */
				if (tmp->get_access_mode() == SD_Workstation::SEQUENTIAL_ACCESS)
				{
					candidate = tmp->pop_fifo();   /* the return value is stored just for debugging */
					XBT_DEBUG("Head of the FIFO: '%s'", (candidate != nullptr) ? candidate->get_name() : "NULL");
					xbt_assert(candidate == candidate_list[i], "Error in __SD_task_just_done: bad first task in the FIFO");
				}
			}

			/* finally execute the task */
			XBT_DEBUG("Task '%s' state: %d", candidate->get_name(), candidate->get_state());
			candidate->really_run();

			XBT_DEBUG("Calling __SD_task_is_running: task '%s', state set: %p, running_task_set: %p, is running: %d",
					candidate->get_name(), candidate->state_set, SD_Simulation::_t->running_task_set, candidate->is_running());
			xbt_assert(candidate->is_running(), "Bad state of task '%s': %d", candidate->get_name(), candidate->get_state());
			XBT_DEBUG("Okay, the task is running.");
		}

		candidate->fifo_checked = 1;
	} /* for each candidate */

	//xbt_free(candidates);
}

void SD_Task::really_run()
{
	void **surf_workstations;

	xbt_assert(is_runnable() || is_in_fifo(), "Task '%s' is not runnable or in a fifo! Task state: %d", name, get_state());

	XBT_DEBUG("Really running task '%s'", name);
	int workstation_nb = workstation_list.size();

	/* set this task as current task for the workstations in sequential mode */
	for (int i = 0; i < workstation_nb; i++)
		if (workstation_list[i]->get_access_mode() == SD_Workstation::SEQUENTIAL_ACCESS)
		{
			workstation_list[i]->set_current_task(this);
			xbt_assert(workstation_list[i]->is_busy(), "The workstation should be busy now");
		}

	XBT_DEBUG("Task '%s' set as current task for its workstations", name);

	/* start the task */

	/* we have to create a Surf workstation array instead of the SimDag
	* workstation array */
	surf_workstations = xbt_new(void *, workstation_nb);

	for (int i = 0; i < workstation_nb; i++)
		surf_workstations[i] = workstation_list[i];

	double *flops_amount = xbt_new0(double, workstation_nb);
	double *bytes_amount = xbt_new0(double, workstation_nb * workstation_nb);


	if (flops_amount)
		memcpy(flops_amount, flops_amount, sizeof(double) * workstation_nb);
	if(bytes_amount)
		memcpy(bytes_amount, bytes_amount, sizeof(double) * workstation_nb * workstation_nb);

	surf_action = surf_workstation_model_execute_parallel_task((surf_workstation_model_t)surf_workstation_model,
																						 workstation_nb,
																						 surf_workstations,
																						 flops_amount,
																						 bytes_amount,
																						 rate);
	surf_action_set_data(surf_action, this);

	XBT_DEBUG("surf_action = %p", surf_action);

	if (category)
		TRACE_surf_action(surf_action, category);

	destroy_scheduling_data();      /* now the scheduling data are not useful anymore */
	set_state(RUNNING);
	xbt_assert(is_running(), "Bad state of task '%s': %d", name, get_state());
}

bool SD_Task::is_scheduled() {return state_set == &SD_Simulation::_t->scheduled_task_set;}
bool SD_Task::is_schedulable() {return state_set == &SD_Simulation::_t->schedulable_task_set;}
bool SD_Task::is_runnable() {return state_set == &SD_Simulation::_t->runnable_task_set;}
bool SD_Task::is_running() {return state_set == &SD_Simulation::_t->running_task_set;}
bool SD_Task::is_in_fifo() {return state_set == &SD_Simulation::_t->in_fifo_task_set;}
