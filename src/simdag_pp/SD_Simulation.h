#pragma once

#include "stdafx.h"
#include "SD_Swag.h"
#include "xbt/dynar.h"
#include "surf/surf.h"

#include "SD_Task.h"
#include "SD_Link.h"

class SD_Simulation
{
public:
	/** Access to simulation, do not use it before calling "init" function below **/
	static SD_Simulation* _t;
private:
	/** Use "init" function instead of constructor **/
	SD_Simulation();
public:
	/** Initializes SD internal data **/
	static void init(int *argc, char **argv);

	/** Set a configuration variable **/
	static void config(const char *key, const char *value);

	/** Reinits the application part of the simulation (experimental feature) **/
	static void application_reinit();

	/** Creates the environment **/
	static void create_environment(const char *platform_file);

	/** Launches the simulation **/
	static std::vector<SD_Task*> simulate(double how_long);
	static std::vector<SD_Task*> simulate_swag(double how_long);

	/** Returns the current clock **/
	static inline double get_clock() {return surf_get_clock();}

	/** Destroys all SD internal data **/
	static void exit();

	/** Loads a DAX file describing a DAG **/
	static xbt_dynar_t daxload(const char *filename);
	/** Loads a DOT file describing a DAG **/
	static xbt_dynar_t dotload(const char *filename);

public:
	/** Array returned by SD_route_get_list and mallocated only once **/
	SD_Link* recyclable_route;
	/* Has a task just reached a watch point ? **/
	int watch_point_reached;

	SD_Swag<SD_Task*> not_scheduled_task_set;
	SD_Swag<SD_Task*> schedulable_task_set;
	SD_Swag<SD_Task*> scheduled_task_set;
	SD_Swag<SD_Task*> runnable_task_set;
	SD_Swag<SD_Task*> in_fifo_task_set;
	SD_Swag<SD_Task*> running_task_set;
	SD_Swag<SD_Task*> done_task_set;
	SD_Swag<SD_Task*> failed_task_set;
	SD_Swag<SD_Task*> return_set;

	int task_number;
};
