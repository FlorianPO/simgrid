#pragma once

#include "stdafx.hpp"

class SD_Storage
{
public:

	/** Creates a storage **/
	static SD_Storage* create(void* surf_storage, void* data);
	/** Deletes a storage **/
	inline static void destroy(void* storage) {delete static_cast<SD_Storage*>(storage);}

	/** Return the host of the storage **/
	inline const char* get_host() {return host;}

private:
	void* data;
	const char *host;
};

