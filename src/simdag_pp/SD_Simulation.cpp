#include "SD_Simulation.h"

#include "SD_Workstation.h"
#include "SD_Storage.h"

XBT_LOG_NEW_CATEGORY(sd_pp, "Logging specific to SimDag");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(sd_pp_kernel, sd_pp, "Logging specific to SimDag (kernel)");

void SD_Simulation::init(int *argc, char **argv)
{
	_t = new SD_Simulation();

	static bool already_called = false;
	if (already_called)
		xbt_assert(already_called == true, "SD_init() already called");
	already_called = true;

	TRACE_global_init(argc, argv);

	SD_Workstation::clear_list();
	SD_Link::clear_list();

	_t->recyclable_route = nullptr;
	_t->watch_point_reached = false;
	_t->task_number = 0;

	surf_init(argc, argv);

	xbt_cfg_setdefault_string(_sg_cfg_set, "workstation/model", "ptask_L07");

	#ifdef HAVE_JEDULE
		jedule_sd_init();
	#endif

	XBT_DEBUG("ADD SD LEVELS");
	SD_HOST_LEVEL = xbt_lib_add_level(host_lib, SD_Workstation::destroy);
	SD_LINK_LEVEL = xbt_lib_add_level(link_lib, SD_Link::destroy);
	SD_STORAGE_LEVEL = xbt_lib_add_level(storage_lib, SD_Storage::destroy);

	if (_sg_cfg_exit_asap)
	{
		SD_exit();
		exit();
	}
}

void SD_Simulation::config(const char *key, const char *value)
{
	xbt_assert(_t, "ERROR: Please call SD_init() before using SD_config()");
	xbt_cfg_set_as_string(_sg_cfg_set, key, value);
}

void SD_Simulation::application_reinit()
{
	XBT_DEBUG("Recreating the swags...");
	_t->not_scheduled_task_set.clear();
	_t->schedulable_task_set.clear();
	_t->scheduled_task_set.clear();
	_t->runnable_task_set.clear();
	_t->in_fifo_task_set.clear();
	_t->running_task_set.clear();
	_t->failed_task_set.clear();

	//TODO free memory

	auto tmp = _t->done_task_set.get_ptr();
	for (int i=0; i < _t->done_task_set.size(); i++)
	{
		if ((*tmp)->tasks_before.size() == 0)
			(*tmp)->set_state(SD_Task::SCHEDULABLE);
		else
		{
			(*tmp)->set_state(SD_Task::NOT_SCHEDULED);
			(*tmp)->set_unsatisfied_dependencies((*tmp)->tasks_before.size());
			(*tmp)->set_is_not_ready((*tmp)->tasks_before.size());
		}
		(*tmp)->workstation_list.clear();
		tmp++;
	}

	_t->done_task_set.clear();
	_t->task_number = 0;

	#ifdef HAVE_JEDULE
		jedule_sd_dump();
		jedule_sd_cleanup();
		jedule_sd_init();
	#endif
}

void SD_Simulation::create_environment(const char *platform_file)
{
	xbt_lib_cursor_t cursor = nullptr;
	char *name = nullptr;
	void **surf_workstation = nullptr;
	void **surf_link = nullptr;
	void **surf_storage = nullptr;

	parse_platform_file(platform_file);

	/* now let's create the SD wrappers for workstations, storages and links */
	xbt_lib_foreach(host_lib, cursor, name, surf_workstation)
	{
		if (surf_workstation[SURF_WKS_LEVEL])
			SD_Workstation::create(surf_workstation[SURF_WKS_LEVEL], nullptr);
	}

	xbt_lib_foreach(link_lib, cursor, name, surf_link)
	{
		if (surf_link[SURF_LINK_LEVEL])
			SD_Link::create(surf_link[SURF_LINK_LEVEL], nullptr);
	}

	xbt_lib_foreach(storage_lib, cursor, name, surf_storage)
	{
		if (surf_storage[SURF_STORAGE_LEVEL])
			SD_Storage::create(surf_storage[SURF_STORAGE_LEVEL], nullptr);
	}

	XBT_DEBUG("Workstation number: %d, link number: %d", int(SD_Workstation::get_list()->size()), int(SD_Link::get_list()->size()));
	#ifdef HAVE_JEDULE
		jedule_setup_platform();
	#endif
}

std::vector<SD_Task*> SD_Simulation::simulate(double how_long)
{
	std::vector<SD_Task*> changed_tasks;
	SD_Task* task;

	simulate_swag(how_long);
	while((task = _t->return_set.pop_back()) != nullptr)
		changed_tasks.push_back(task);

	return changed_tasks;
}

std::vector<SD_Task*> SD_Simulation::simulate_swag(double how_long)
{
	double total_time = 0.0;      /* we stop the simulation when total_time >= how_long */
	double elapsed_time = 0.0;
	surf_action_t action;
	unsigned int iter;
	SD_Task* task;

	static bool first_time = true;
	if (first_time)
	{
	    XBT_VERB("Starting simulation...");
	    surf_presolve();            /* Takes traces into account */
	    first_time = false;
	}

	XBT_VERB("Run simulation for %f seconds", how_long);
	_t->watch_point_reached = false;
	_t->return_set.clear();

	/* explore the runnable tasks */
	auto tmp = _t->runnable_task_set.get_ptr();
	for (int i=0; i < _t->runnable_task_set.size(); i++)
	{
		XBT_VERB("Executing task '%s'", (*tmp)->get_name());
		if ((*tmp)->try_to_run())
			_t->return_set.push_back(*tmp);
		tmp++;
	}

	/* main loop */
	elapsed_time = 0.0;
	while (elapsed_time >= 0.0 && (how_long < 0.0 || 0.00001 < (how_long - total_time)) && !_t->watch_point_reached)
	{
	    surf_model_t model = nullptr;
	    /* dumb variables */
	    XBT_DEBUG("Total time: %f", total_time);

	    elapsed_time = surf_solve(how_long > 0 ? surf_get_clock() + how_long - total_time : -1.0);
	    XBT_DEBUG("surf_solve() returns %f", elapsed_time);
	    if (elapsed_time > 0.0)
	    	total_time += elapsed_time;

	    /* FIXME: shoud look at model_list or model_list_invoke? */
	    /* let's see which tasks are done */
	    xbt_dynar_foreach(model_list, iter, model)
	    {
	    	while ((action = surf_model_extract_done_action_set(model)))
	    	{
	    		task = static_cast<SD_Task*>(surf_action_get_data(action));
	    		task->start_time = surf_action_get_start_time(task->surf_action);

				task->finish_time = surf_get_clock();
				XBT_VERB("Task '%s' done", task->get_name());
				XBT_DEBUG("Calling __SD_task_just_done");
				task->just_done();
				XBT_DEBUG("__SD_task_just_done called on task '%s'", task->get_name());

				/* the state has changed */
				_t->return_set.push_back(task);

				/* remove the dependencies after this task */
				SD_Task* tmp;
				for (unsigned int i=0; i < task->tasks_after.size(); i++)
				{
					tmp = task->tasks_after[i]->getDst();
					if (tmp->unsatisfied_dependencies > 0)
						tmp->unsatisfied_dependencies--;
					if (tmp->is_not_ready > 0)
						tmp->is_not_ready--;

					XBT_DEBUG("Released a dependency on %s: %d remain(s). Became schedulable if %d=0",
							tmp->get_name(), tmp->unsatisfied_dependencies, tmp->is_not_ready);

					if (!tmp->unsatisfied_dependencies)
					{
						if (tmp->is_scheduled())
							tmp->set_state(SD_Task::RUNNABLE);
						else
							tmp->set_state(SD_Task::SCHEDULABLE);
					}

					if (!tmp->is_scheduled() && !tmp->is_not_ready)
						tmp->set_state(SD_Task::SCHEDULABLE);

					if (tmp->get_kind() == SD_Task::COMM_E2E)
					{
						SD_Task_Dependency* comm_dep = tmp->tasks_after[0];
						SD_Task* comm_dst = comm_dep->getDst();

						if (!comm_dst->is_scheduled() && comm_dst->is_not_ready > 0)
						{
							comm_dst->is_not_ready--;

							XBT_DEBUG("%s is a transfer, %s may be ready now if %d=0", tmp->get_name(), comm_dst->get_name(), comm_dst->is_not_ready);

							if (!comm_dst->is_not_ready)
								comm_dst->set_state(SD_Task::SCHEDULABLE);
						}
					}

					/* is dst runnable now? */
					if (tmp->is_runnable() && !_t->watch_point_reached)
					{
						XBT_VERB("Executing task '%s'", tmp->get_name());
						if (tmp->try_to_run())
							_t->return_set.push_back(tmp);
					}
				}
	    	}

	    	/* let's see which tasks have just failed */
	    	while ((action = surf_model_extract_failed_action_set(model)))
	    	{
	    		task = static_cast<SD_Task*>(surf_action_get_data(action));
	    		task->start_time = surf_action_get_start_time(task->surf_action);
	    		task->finish_time = surf_get_clock();
	    		XBT_VERB("Task '%s' failed", task->get_name());
	    		task->set_state(SD_Task::FAILED);
	    		surf_action_unref(action);
	    		task->surf_action = nullptr;

	    		_t->return_set.push_back(task);
	    	}
	    }
	}

	if (!_t->watch_point_reached && how_long < 0)
	    if (_t->done_task_set.size() < _t->task_number)
	    {
	        XBT_WARN("Simulation is finished but %d tasks are still not done", _t->task_number - _t->done_task_set.size());

	        tmp = _t->not_scheduled_task_set.get_ptr();
	        for (int i=0; _t->not_scheduled_task_set.size(); i++)
	        {
	        	XBT_WARN("%s is in NOT_SCHEDULED state", (*tmp)->get_name());
	        	tmp++;
	        }
	        tmp = _t->schedulable_task_set.get_ptr();
	        for (int i=0; _t->not_scheduled_task_set.size(); i++)
	        {
	        	XBT_WARN("%s is in SCHEDULABLE state", (*tmp)->get_name());
	        	tmp++;
	        }
	        tmp = _t->scheduled_task_set.get_ptr();
			for (int i=0; _t->not_scheduled_task_set.size(); i++)
			{
				XBT_WARN("%s is in SCHEDULED state", (*tmp)->get_name());
				tmp++;
			}
	    }

	XBT_DEBUG("elapsed_time = %f, total_time = %f, _t->watch_point_reached = %d", elapsed_time, total_time, _t->watch_point_reached);
	XBT_DEBUG("current time = %f", surf_get_clock());

	return *_t->return_set.get_vector();
}

void SD_Simulation::exit()
{
	TRACE_surf_resource_utilization_release();

	XBT_DEBUG("Destroying workstation and link arrays...");
	for (unsigned int i=0; i < SD_Workstation::get_list()->size(); i++)
		delete &SD_Workstation::get_list()[i];
	for (unsigned int i=0; i < SD_Link::get_list()->size(); i++)
		delete &SD_Link::get_list()[i];

	//xbt_free(sd_global->recyclable_route);

	XBT_DEBUG("Destroying the swags...");
	TRACE_end();

	delete _t;
	_t = nullptr;

	#ifdef HAVE_JEDULE
		jedule_sd_dump();
		jedule_sd_cleanup();
		jedule_sd_exit();
	#endif

	XBT_DEBUG("Exiting Surf...");
	surf_exit();
}
