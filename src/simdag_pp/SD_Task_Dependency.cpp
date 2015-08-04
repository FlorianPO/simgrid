#include "SD_Task_Dependency.hpp"

void SD_Task_Dependency::add(char* name, void* data, SD_Task* src, SD_Task* dst)
{
	SD_Task_Dependency* tmp = new SD_Task_Dependency();
	tmp->name = name;
	tmp->data = data;
	tmp->src = src;
	tmp->dst = dst;

	dependency_list.push_back(tmp);
}

void SD_Task_Dependency::remove(SD_Task* src, SD_Task* dst)
{
	for (unsigned int i=0; i < dependency_list.size(); i++)
		if (dependency_list[i]->getSrc() == src && dependency_list[i]->getDst() == dst)
		{
			dependency_list[i]->remove();
			return;
		}
}

void SD_Task_Dependency::remove(SD_Task* task)
{
	if (task == nullptr)
		delete this;
	else
	{
		if (src == task)
			src = nullptr;
		else
			dst = nullptr;

		if (src == nullptr && dst == nullptr)
			delete this;
	}
}

bool SD_Task_Dependency::exists(SD_Task* src, SD_Task* dst)
{
	for (unsigned int i=0; i < dependency_list.size(); i++)
		if (dependency_list[i]->getSrc() == src && dependency_list[i]->getDst() == dst)
			return true;

	return false;
}
