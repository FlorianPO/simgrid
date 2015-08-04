#include "SD_Route.hpp"

#include "SD_Simulation.hpp"

SD_Simulation* SD_Simulation::_t;

SD_Link* SD_Route::get_list(SD_Workstation* src, SD_Workstation* dst)
{
	xbt_dynar_t surf_route;
	const char *link_name;
	void *surf_link;
	unsigned int cpt;

	if (SD_Simulation::_t->recyclable_route == nullptr) /* first run */
		SD_Simulation::_t->recyclable_route = xbt_new(SD_Link, SD_Link::get_list()->size());

	surf_route = surf_workstation_model_get_route(static_cast<surf_workstation_model_t>(surf_workstation_model),
			static_cast<surf_resource_t>(src->get_surf()), static_cast<surf_resource_t>(dst->get_surf()));

	xbt_dynar_foreach(surf_route, cpt, surf_link)
	{
		link_name = surf_resource_name(static_cast<Resource*>(surf_link));
		memcpy(SD_Simulation::_t->recyclable_route + cpt, xbt_lib_get_or_null(link_lib, link_name, SD_LINK_LEVEL), int(sizeof(SD_Link*)));
	}
	return SD_Simulation::_t->recyclable_route;
}

int SD_Route::get_size(SD_Workstation* src, SD_Workstation* dst)
{
	  return xbt_dynar_length(surf_workstation_model_get_route(static_cast<surf_workstation_model_t>(surf_workstation_model),
				static_cast<surf_resource_t>(src->get_surf()), static_cast<surf_resource_t>(dst->get_surf())));
}

double SD_Route::get_current_latency(SD_Workstation* src, SD_Workstation* dst)
{
	SD_Link* links = get_list(src, dst);
	double latency = 0.0;

	SD_Link* tmp;
	for (int i = 0; i < get_size(src, dst); i++)
	{
		tmp = links+i;
		latency += tmp->get_current_latency();
	}

	return latency;
}

double SD_Route::get_current_bandwidth(SD_Workstation* src, SD_Workstation* dst)
{
	SD_Link* links = get_list(src, dst);
	double min_bandwidth = -1.0;
	double bandwidth;

	SD_Link* tmp;
	for (int i = 0; i < get_size(src, dst); i++)
	{
		tmp = links+i;
		bandwidth = tmp->get_current_bandwidth();
		if (bandwidth < min_bandwidth || min_bandwidth == -1.0)
			min_bandwidth = bandwidth;
	}

  return min_bandwidth;
}

double SD_Route::get_communication_time(SD_Workstation* src, SD_Workstation* dst, double bytes_amount)
{
	/* total time = latency + transmission time of the slowest link transmission time of a link = communication amount / link bandwidth */

	double bandwidth, min_bandwidth;
	double latency;

	xbt_assert(bytes_amount >= 0, "bytes_amount must be greater than or equal to zero");

	if (bytes_amount == 0.0)
		return 0.0;

	SD_Link* links = get_list(src, dst);
	min_bandwidth = -1.0;
	latency = 0;

	SD_Link* tmp;
	for (int i = 0; i < get_size(src, dst); i++)
	{
		tmp = links+i;
		latency += tmp->get_current_latency();
		bandwidth = tmp->get_current_bandwidth();
		if (bandwidth < min_bandwidth || min_bandwidth == -1.0)
			min_bandwidth = bandwidth;
	}

	return latency + (bytes_amount / min_bandwidth);
}

