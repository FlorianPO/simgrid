#include "SD_Simulation.h"
#include "xbt/sysdep.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(sd_test, "Logging specific to this SimDag example");

int main(int argc, char **argv)
{
	xbt_ex_t ex;
	const char *platform_file;
	double rate = -1.0;

	/* initialization of SD */
	SD_Simulation::init(&argc, argv);

	/*  xbt_log_control_set("sd.thres=debug"); */
	if (argc < 2)
	{
		XBT_INFO("Usage: %s platform_file", argv[0]);
		XBT_INFO("example: %s sd_platform.xml", argv[0]);
		exit(1);
	}

	/* creation of the environment */
	platform_file = argv[1];
	SD_Simulation::create_environment(platform_file);

	/* test the estimation functions */
	SD_Workstation* w1 = SD_Workstation::get_list()[0];
	SD_Workstation* w2 = SD_Workstation::get_list()[1];
	w2->set_access_mode(SD_Workstation::SEQUENTIAL_ACCESS);

	std::vector<SD_Workstation*> workstation_list {w1, w2};

	double computation_amount1 = 2000000;
	double computation_amount2 = 1000000;
	double communication_amount12 = 2000000;
	double communication_amount21 = 3000000;
	XBT_INFO("Computation time for %f flops on %s: %f",
			computation_amount1, w1->get_name(), w1->get_computation_time(computation_amount1));
	XBT_INFO("Computation time for %f flops on %s: %f",
			computation_amount2, w2->get_name(), w2->get_computation_time(computation_amount2));

	XBT_INFO("Route between %s and %s:", w1->get_name(), w2->get_name());

	auto route = SD_Route::get_list(w1, w2);
	SD_Link* tmp;
	for (int i = 0; i < SD_Route::get_size(w1, w2); i++)
	{
		tmp = route[i];
		XBT_INFO("   Link %s: latency = %f, bandwidth = %f", tmp->get_name(), tmp->get_current_latency(), tmp->get_current_bandwidth());
	}

	XBT_INFO("Route latency = %f, route bandwidth = %f",
			SD_Route::get_current_latency(w1, w2), SD_Route::get_current_bandwidth(w1, w2));
	XBT_INFO("Communication time for %f bytes between %s and %s: %f",
			communication_amount12, w1->get_name(), w1->get_name(), SD_Route::get_communication_time(w1, w2, communication_amount12));
	XBT_INFO("Communication time for %f bytes between %s and %s: %f",
		communication_amount21, w1->get_name(), w1->get_name(), SD_Route::get_communication_time(w2, w1, communication_amount21));

	/* creation of the tasks and their dependencies */
	SD_Task* taskA = SD_Task::create("Task A", nullptr, 10.0);
	SD_Task* taskB = SD_Task::create("Task B", nullptr, 40.0);
	SD_Task* taskC = SD_Task::create("Task C", nullptr, 30.0);
	SD_Task* taskD = SD_Task::create("Task D", nullptr, 60.0);

	/* try to attach and retrieve user data to a task */
	taskA->set_data(&computation_amount1);
	if (computation_amount1 != (*((double*) taskA->get_data())))
		XBT_ERROR("User data was corrupted by a simple set/get");

	SD_Task_Dependency::add(nullptr, nullptr, taskB, taskA);
	SD_Task_Dependency::add(nullptr, nullptr, taskC, taskA);
	SD_Task_Dependency::add(nullptr, nullptr, taskD, taskB);
	SD_Task_Dependency::add(nullptr, nullptr, taskD, taskC);
	/*  SD_task_dependency_add(NULL, NULL, taskA, taskD); /\* deadlock */

	TRY {
		SD_Task_Dependency::add(nullptr, nullptr, taskA, taskA);   /* shouldn't work and must raise an exception */
		xbt_die("Hey, I can add a dependency between Task A and Task A!");
	}
	CATCH(ex) {
		if (ex.category != arg_error)
			RETHROW;                  /* this is a serious error */
		xbt_ex_free(ex);
	}

	TRY {
		SD_Task_Dependency::add(nullptr, nullptr, taskB, taskA);   /* shouldn't work and must raise an exception */
		xbt_die("Oh oh, I can add an already existing dependency!");
	}
	CATCH(ex) {
		if (ex.category != arg_error)
			RETHROW;
		xbt_ex_free(ex);
	}

	TRY {
		SD_Task_Dependency::remove(taskA, taskC);    /* shouldn't work and must raise an exception */
		xbt_die("Dude, I can remove an unknown dependency!");
	}
	CATCH(ex)
	{
		if (ex.category != arg_error)
			RETHROW;
		xbt_ex_free(ex);
	}

	TRY
	{
		SD_Task_Dependency::remove(taskC, taskC);    /* shouldn't work and must raise an exception */
		xbt_die("Wow, I can remove a dependency between Task C and itself!");
	}
	CATCH(ex)
	{
		if (ex.category != arg_error)
			RETHROW;
		xbt_ex_free(ex);
	}


	/* if everything is ok, no exception is forwarded or rethrown by main() */

	/* watch points */
	taskD->watch(SD_Task::DONE);
	taskB->watch(SD_Task::DONE);
	taskD->unwatch(SD_Task::DONE);

	/* scheduling parameters */
	double computation_amount[2];
	computation_amount[0] = computation_amount1;
	computation_amount[1] = computation_amount2;
	double communication_amount[2];
	communication_amount[1] = communication_amount12;
	communication_amount[2] = communication_amount21;

	/* estimated time */
	SD_Task* task = taskD;
	XBT_INFO("Estimated time for '%s': %f",
			task->get_name(), task->get_execution_time(computation_amount, communication_amount));

	/* let's launch the simulation! */

	taskA->schedule(&workstation_list, computation_amount, communication_amount, rate);
	taskB->schedule(&workstation_list, computation_amount, communication_amount, rate);
	taskC->schedule(&workstation_list, computation_amount, communication_amount, rate);
	taskD->schedule(&workstation_list, computation_amount, communication_amount, rate);

	auto changed_tasks = SD_Simulation::simulate(-1.0);
	for (int i=0; i < changed_tasks.size(); i++)
	{
		task = changed_tasks[i];
		XBT_INFO("Task '%s' start time: %f, finish time: %f", task->get_name(), task->get_start_time(), task->get_finish_time());
	}

	auto checkD = changed_tasks[0];
	auto checkB = changed_tasks[1];

	xbt_assert(checkD == taskD && checkB == taskB, "Unexpected simulation results");

	XBT_DEBUG("Destroying tasks...");

	taskA->destroy();
	taskB->destroy();
	taskC->destroy();
	taskD->destroy();

	XBT_DEBUG("Tasks destroyed. Exiting SimDag...");

	SD_Simulation::exit();
	return 0;
}
