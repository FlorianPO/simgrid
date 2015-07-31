#pragma once

#include "stdafx.h"
#include "SD_Workstation.h"
#include "SD_Link.h"

class SD_Route
{
public:
	/** Returns a link of links as the route between workstations src and dst **/
	static SD_Link* get_list(SD_Workstation* src, SD_Workstation* dst);
	/** Returns the length of the route between workstations src and dst **/
	static int get_size(SD_Workstation* src, SD_Workstation* dst);

	/** Returns the current latency of the route between workstations src and dst **/
	static double get_current_latency(SD_Workstation* src, SD_Workstation* dst);
	/** Returns the current bandwidth of the route between workstations src and dst **/
	static double get_current_bandwidth(SD_Workstation* src, SD_Workstation* dst);
	/** Returns the communication time of the route between workstations src and dst **/
	static double get_communication_time(SD_Workstation* src, SD_Workstation* dst, double bytes_amount);
};
