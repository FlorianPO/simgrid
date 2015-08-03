#include "SD_Workstation.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(SD_Workstation, sd, "Logging specific to SimDag (workstation)");

SD_Workstation* SD_Workstation::create(void* surf_workstation, void* data)
{
	SD_Workstation* work;
	const char* name;

	work = new SD_Workstation();
	work->data = data; //user data

	name = surf_resource_name(static_cast<Resource*>(surf_workstation));
	xbt_lib_set(host_lib, name, SD_HOST_LEVEL, work);

	work->workstation_surf = xbt_lib_get_elm_or_null(host_lib, name);
	return work;
}

SD_Workstation* SD_Workstation::get_by_name(char* name) {return static_cast<SD_Workstation*>(xbt_lib_get_level(xbt_lib_get_elm_or_null(host_lib, name), SD_HOST_LEVEL));}

char* SD_Workstation::get_name() {return sg_host_name(workstation_surf);}

void SD_Workstation::set_data(void* data) {this->data = data;}

void* SD_Workstation::get_data() {return data;}

double SD_Workstation::get_power() {return surf_workstation_get_speed(workstation_surf, 1.0);}

double SD_Workstation::get_available_power() {return surf_workstation_get_available_speed(workstation_surf);}

int SD_Workstation::get_cores() {return surf_workstation_get_core(workstation_surf);}

SD_Workstation::ACCESS_MODE SD_Workstation::get_access_mode() {return access_mode;}

void SD_Workstation::set_access_mode(ACCESS_MODE access_mode) {this->access_mode = access_mode;}

xbt_dict_t SD_Workstation::get_properties() {return surf_resource_get_properties(static_cast<Resource*>(surf_workstation_resource_priv(workstation_surf)));}

const char* SD_Workstation::get_property_value(const char* name) {return static_cast<const char*>(xbt_dict_get_or_null(get_properties(), name));}

SD_Task* SD_Workstation::get_current_task()
{
	xbt_assert(access_mode == SEQUENTIAL_ACCESS, "Access mode must be set to SD_WORKSTATION_SEQUENTIAL_ACCESS to use this function");
	return current_task;
}

xbt_dict_t SD_Workstation::get_mounted_storage_list() {return surf_workstation_get_mounted_storage_list(workstation_surf);}

xbt_dynar_t SD_Workstation::get_attached_storage_list() {return surf_workstation_get_attached_storage_list(workstation_surf);}

double SD_Workstation::get_computation_time(double flops_amount)
{
	xbt_assert(flops_amount >= 0, "flops_amount must be greater than or equal to zero");
	return flops_amount / get_power();
}

void SD_Workstation::dump()
{
	xbt_dict_t props;
	xbt_dict_cursor_t cursor = nullptr;
	char *key, *data;
	SD_Task* task = nullptr;

	XBT_INFO("Displaying workstation %s", get_name());
	XBT_INFO("  - power: %.0f", get_power());
	XBT_INFO("  - available power: %.2f", get_available_power());
	switch (access_mode)
	{
		case SHARED_ACCESS:
			XBT_INFO("  - access mode: Space shared"); break;
		case SEQUENTIAL_ACCESS:
			XBT_INFO("  - access mode: Exclusive");
			task = get_current_task();
			if (task)
				XBT_INFO("    current running task: %s", task->get_name());
			else
				XBT_INFO("    no task running"); break;
		default:
			break;
	}
	props = get_properties();

	if (!xbt_dict_is_empty(props))
	{
		XBT_INFO("  - properties:");
		xbt_dict_foreach(props, cursor, key, data) {XBT_INFO("    %s->%s",key,data);}
	}
}

bool SD_Workstation::is_busy()
{
	 XBT_DEBUG ("Workstation '%s' access mode: '%s', current task: %s, task_fifo size: %d", get_name(),
	       (access_mode == SHARED_ACCESS) ? "SHARED" : "FIFO",
	       (current_task ? current_task->get_name() : "none"),
	       int(task_fifo.size()));

	 return access_mode == SEQUENTIAL_ACCESS && (current_task != nullptr || task_fifo.size() > 0);
}
