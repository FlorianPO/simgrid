#pragma once

#include <vector>

template <typename T>
class SD_Swag
{
public:
	/** If you know how many objects your program will contain, you may indicate it here for more performance */
	inline SD_Swag(int reserve = 0) {if (reserve > 0) swag_vector.reserve(reserve);}

	inline T* get_ptr() {return &swag_vector[0];} //Returns a pointer at the beginning of the swag
	inline std::vector<T>* get_vector() {return &swag_vector;}
	inline void push_back(T object) {swag_vector.push_back(object);} //Adds an object at the end

	/** Erases an object by replacing it with the last object (no reallocation) */
	void remove(T object)
	{
		for (int i=0; i < swag_vector.size()-1; i++) //The last element doesnt'need to be checked
			if (swag_vector[i] == object)
			{
				swag_vector[i] = swag_vector[swag_vector.size()-1];
				break;
			}
		swag_vector.erase(swag_vector.begin()+swag_vector.size()-1); //Erase last element
	}

	inline void clear() {swag_vector.clear();}	//Empties the swag
	inline int size() {return swag_vector.size();}

	T pop_back() //Returns last object
	{
		T tmp = swag_vector.back();
		swag_vector.erase(swag_vector.begin()+swag_vector.size()-1); //Erase last element
		return tmp;
	}

private:
	std::vector<T> swag_vector;
};
