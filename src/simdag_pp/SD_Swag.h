#pragma once

#include <vector>

template <typename T>
class SD_Swag
{
public:
	SD_Swag(int reserve = 0); 	//If you know how many objects your program will contain, you may indicate it here for more performance
	~SD_Swag();

	T* get_ptr(); 				//Returns a pointer at the beginning of the swag
	std::vector<T>* get_vector();
	void push_back(T object); 	//Adds an object at the end
	void remove(T object); 		//Erases an object by replacing it with the last object (no reallocation)
	void clear(); 				//Empties the swag
	int size();

	T pop_back();				//Returns last object

private:
	std::vector<T> swag_vector;
};
