#pragma once

#include "SD_Task.h"
#include "stdafx.h"

class SD_Task;

class SD_Workstation
{
public:
	enum ACCESS_MODE
	{
		SHARED_ACCESS,    									/**< @brief Several tasks can be executed at the same time */
		SEQUENTIAL_ACCESS  									/**< @brief Only one task can be executed, the others wait in a FIFO */
	};

	/** Creates a workstation and registers it in SD */
	static SD_Workstation* create(void* surf_workstation, void* data);
	/** Deletes a workstation */
	static void destroy(void* workstation) {delete static_cast<SD_Workstation*>(workstation);}

	/** Returns the workstation list */
	inline static const std::vector<SD_Workstation*>* get_list() {return &workstation_list;}
	/** Clear the workstation list */
	inline static void clear_list() {workstation_list.clear();}

	/** Returns a workstation given its name, NULL if not found */
	static SD_Workstation* get_by_name(char* name);

	/** Returns the name of this workstation */
	char* get_name();

	/** Sets the user data of this workstation, the old data should have been freed first if it was not NULL */
	void set_data(void* data);
	/** Returns the user data of this workstation */
	void* get_data();

	/** Returns the total power of this workstation */
	double get_power();
	/** Returns the proportion of available power in this workstation */
	double get_available_power();
	/** Returns the amount of cores of this workstation */
	int get_cores();

	/** Returns the access mode of this workstation */
	ACCESS_MODE get_access_mode();
	/** Sets the access mode for the tasks that will be executed on this workstation */
	void set_access_mode(ACCESS_MODE access_mode);

	/** Returns a xbt_dict_t consisting of the list of properties assigned to this workstation */
	xbt_dict_t get_properties();
	/** Returns the value of a given workstation property */
	const char* get_property_value(const char* name);

	/** Returns the kind of the task currently running on this workstation. Only call this with sequential access mode set */
	SD_Task* get_current_task();
	inline void set_current_task(SD_Task* tmp) {current_task = tmp;}
	/** Return the list of mounted storages on this workstation */
	xbt_dict_t get_mounted_storage_list();
	xbt_dynar_t get_attached_storage_list();

	/** Returns an approximative estimated time for the given computation amount on this workstation */
	double get_computation_time(double computation_amout);

	/** Displays debugging informations about this workstation */
	void dump();

	bool is_busy();

	inline SD_workstation_t get_surf() {return workstation_surf;}
	inline void push_fifo(SD_Task* tmp) {task_fifo.push(tmp);}
	inline SD_Task* pop_fifo()
	{
		SD_Task* tmp = task_fifo.front();
		task_fifo.pop();
		return tmp;
	}
	inline SD_Task* front_fifo() {return task_fifo.front();}

private:
	/** A vector of SD_Workstation* containing all workstations */
	static std::vector<SD_Workstation*> workstation_list;

	SD_workstation_t workstation_surf;

	std::queue<SD_Task*> task_fifo;
	SD_Task* current_task = nullptr;
	/** User data of the workstation */
	void* data;

	ACCESS_MODE access_mode = SHARED_ACCESS;
};
