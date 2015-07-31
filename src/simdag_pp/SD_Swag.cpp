#include "SD_Swag.h"

template <typename T>
SD_Swag<T>::SD_Swag(int reserve)
{
	if (reserve > 0)
		swag_vector.reserve(reserve);
}

template <typename T>
SD_Swag<T>::~SD_Swag()
{
}

template <typename T>
void SD_Swag<T>::push_back(T object)
{
	swag_vector.push_back(object);
}

template <typename T>
void SD_Swag<T>::remove(T object)
{
	for (int i=0; i < swag_vector.size()-1; i++) //The last element doesnt'need to be checked
		if (swag_vector[i] == object)
		{
			swag_vector[i] = swag_vector[swag_vector.size()-1];
			break;
		}

	swag_vector.erase(swag_vector.begin()+swag_vector.size()-1); //Erase last element
}

template <typename T>
void SD_Swag<T>::clear()
{
	swag_vector.clear();
}

template <typename T>
T* SD_Swag<T>::get_ptr()
{
	return &swag_vector[0];
}

template <typename T>
int SD_Swag<T>::size()
{
	return swag_vector.size();
}

template <typename T>
T SD_Swag<T>::pop_back()
{
	T tmp = swag_vector.back();
	swag_vector.erase(swag_vector.begin()+swag_vector.size()-1); //Erase last element
	return tmp;
}

template <typename T>
std::vector<T>* SD_Swag<T>::get_vector()
{
	return &swag_vector;
}

