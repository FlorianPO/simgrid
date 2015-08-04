#include "SD_Link.hpp"

SD_Link* SD_Link::create(void *surf_link, void *data)
{
	SD_Link* link = new SD_Link();
	const char* name;

	link->surf_link = surf_link;
	link->data = data; //user data
	if (surf_network_link_is_shared(static_cast<Resource*>(surf_link)))
		link->sharing_policy = SHARED;
	else
		link->sharing_policy = FATPIPE;

	name = link->get_name();
	xbt_lib_set(link_lib, name, SD_LINK_LEVEL, link);

	return link;
}

const char* SD_Link::get_name() {return surf_resource_name(static_cast<Resource*>(surf_link));}

void* SD_Link::get_data() {return data;}

void SD_Link::set_data(void* data) {this->data = data;}

double SD_Link::get_current_bandwidth() {return surf_network_link_get_bandwidth(static_cast<Resource*>(surf_link));}

double SD_Link::get_current_latency() {return surf_network_link_get_latency(static_cast<Resource*>(surf_link));}

SD_Link::SHARING_POLICY SD_Link::get_sharing_policy() {return sharing_policy;}
