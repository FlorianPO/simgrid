#pragma once

#include "stdafx.hpp"

class SD_Link
{
public:
	/** Sharing_policy of the link **/
	enum SHARING_POLICY
	{
		SHARED,
		FATPIPE
	};

	/** Deletes a link **/
	static void destroy(void* link) {delete static_cast<SD_Link*>(link);}
	/** Creates a link and registers it in SD **/
	static SD_Link* create(void *surf_link, void *data);

	/** Returns the link vector **/
	static const std::vector<SD_Link*>* get_list() {return &link_list;}
	/** Clear the link vector (the links aren't deleted) **/
	inline static void clear_list() {link_list.clear();}

	/** Returns the name of the link **/
	const char* get_name();
	/** Returns the user data of the link **/
	void* get_data();
	/** Sets the user data of the link **/
	void set_data(void* data);

	/** Returns the current bandwidth of the link **/
	double get_current_bandwidth();
	/** Returns the current latency of the link **/
	double get_current_latency();

	/** Returns the sharing policy of the link **/
	SHARING_POLICY get_sharing_policy();

private:
	/** The vector of SD_Link* containing all links created **/
	static std::vector<SD_Link*> link_list;

	/** User data of the link **/
	void* data;
	/** Surf representative of the link	**/
	void* surf_link;

	/** Sharing policy of the link **/
	SHARING_POLICY sharing_policy;
};
