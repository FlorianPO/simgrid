#pragma once

#include "SD_Task.hpp"
class SD_Task;

class SD_Task_Dependency
{
public:
	/** Adds a dependency between two tasks */
	static void add(char* name, void* data, SD_Task* src, SD_Task* dst);
	/** Remove a dependency between two tasks */
	static void remove(SD_Task* src, SD_Task* dst);
	/** Indicates whether there is a dependency between two tasks */
	static bool exists(SD_Task* src, SD_Task* dst);

	/** Task argument used to remove safely dependency when another task has it too,
	 *  destroy at both side is needed to perform the real destruction.
	 *  If the task is not specified, the dependency will be destroyed anyway */
	void remove(SD_Task* task = nullptr);

	inline char* get_name(SD_Task* src, SD_Task* dst) {return name;}
	inline void* get_data(SD_Task* src, SD_Task* dst) {return data;}

	inline SD_Task* getSrc() {return src;}
	inline SD_Task* getDst() {return dst;}

private:
	static std::vector<SD_Task_Dependency*> dependency_list;

	static void destroy(SD_Task_Dependency* task_dependency) {task_dependency->~SD_Task_Dependency();}

	char* name;
	void* data;
	SD_Task* src;
	SD_Task* dst;
};
