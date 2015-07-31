#include "SD_Storage.h"

#include "surf/surf.h"

SD_Storage* SD_Storage::create(void* surf_storage, void* data)
{
	SD_Storage* storage = new SD_Storage();
	storage->data = data;     /* user data */

	const char *name;
	name = surf_resource_name(static_cast<Resource*>(surf_storage));
	storage->host = surf_storage_get_host(static_cast<surf_resource_t>(surf_storage_resource_by_name(name)));
	xbt_lib_set(storage_lib,name, SD_STORAGE_LEVEL, storage);

	return storage;
}
