#ifndef STATICDATALIST_HPP_
#define STATICDATALIST_HPP_

#include <stdio.h>
#include <memory.h>

constexpr int MAX_STATIC_DATA_LIST_LENGTH = 5;
constexpr int MAX_STATIC_DATA_ELEMENT_LENGTH = 128;

/* StaticDataList - Provides a simple mechanism to store data
 *                  in a static object for the transport through a rtos queue.
 *                  The store method returns a pointer to a pointer to the string
 *                  which could be directly passed as a queue data parameter. When receiving
 *                  the data from the queue the data must be cleared by invoking the delete
 *                  method and passing the data pointer received from the queue.
 * */

template <typename T>
class StaticDataList
{
public:
	StaticDataList();

	T ** Store(const T * data, int size);
	void Delete(T * addr);

	int getDataSize(T * addr);
	int getAvailableCount();
	bool hasFreeSpace();

	void eraseAll();

private:

	class S_EL
	{
	public:
		T tData[MAX_STATIC_DATA_ELEMENT_LENGTH];
		int size;
		uint32_t used;
		T * pData;
	};

	S_EL list[MAX_STATIC_DATA_LIST_LENGTH];
	void resetIndex(int index);
};

template<typename T>
StaticDataList<T>::StaticDataList()
{
	for(int i = 0; i < MAX_STATIC_DATA_LIST_LENGTH; i++)
	{
		this->list[i].size = 0;
		this->list[i].used = 0;
		memset(this->list[i].tData, 0, sizeof(this->list[i].tData));
		this->list[i].pData = this->list[i].tData;
	}
}

template<typename T>
inline T ** StaticDataList<T>::Store(const T * data, int size)
{
	T ** dataPtr = nullptr;

	if(data != nullptr)
	{
		for(int dataListIndex = 0; dataListIndex < MAX_STATIC_DATA_LIST_LENGTH; dataListIndex++)
		{
			if(this->list[dataListIndex].used == 0)
			{
				this->list[dataListIndex].used = 1U;

				bool copySuccess = false;
				this->list[dataListIndex].size = size;

				for(int dataIndexPos = 0; dataIndexPos < MAX_STATIC_DATA_ELEMENT_LENGTH; dataIndexPos++)
				{
					this->list[dataListIndex].tData[dataIndexPos] = data[dataIndexPos];

					if(dataIndexPos >= (size - 1))
					{
						copySuccess = true;
						break;
					}
					else
					{
						if(dataIndexPos == MAX_STATIC_DATA_ELEMENT_LENGTH - 1)
						{
							break;
						}
					}
				}
				if(copySuccess == false)
				{
					this->resetIndex(dataListIndex);
				}
				else
				{
					dataPtr = &this->list[dataListIndex].pData;
				}
				break;
			}
		}
	}
	return dataPtr;
}

template<typename T>
inline void StaticDataList<T>::Delete(T * addr)
{
	if(addr != nullptr)
	{
		for(int dataListIndex = 0; dataListIndex < MAX_STATIC_DATA_LIST_LENGTH; dataListIndex++)
		{
			if(addr == this->list[dataListIndex].tData)
			{
				this->resetIndex(dataListIndex);
				return;
			}
		}
	}
}

template<typename T>
inline int StaticDataList<T>::getDataSize(T * addr)
{
	if(addr != nullptr)
	{
		for(int dataListIndex = 0; dataListIndex < MAX_STATIC_DATA_LIST_LENGTH; dataListIndex++)
		{
			if(addr == this->list[dataListIndex].tData)
			{
				return this->list[dataListIndex].size;
			}
		}
	}
	return -1;
}

template<typename T>
inline void StaticDataList<T>::eraseAll()
{
	for(int dataListIndex = 0; dataListIndex < MAX_STATIC_DATA_LIST_LENGTH; dataListIndex++)
	{
		this->resetIndex(dataListIndex);
	}
}

template<typename T>
inline void StaticDataList<T>::resetIndex(int index)
{
	if(index >= 0 && index < MAX_STATIC_DATA_LIST_LENGTH)
	{
		for(int i = 0; i < this->list[index].size; i++)
		{
			this->list[index].tData[i] = 0;
		}
		this->list[index].size = 0;
		this->list[index].used = 0;
	}
}

template<typename T>
inline int StaticDataList<T>::getAvailableCount()
{
	int count = 0;

	for(int i = 0; i < MAX_STATIC_DATA_LIST_LENGTH; i++)
	{
		if(this->list[i].used == 0)
		{
			count++;
		}
	}
	return count;
}

template<typename T>
inline bool StaticDataList<T>::hasFreeSpace()
{
	return (this->getAvailableCount() != 0);
}

#endif /* STATICDATALIST_HPP_ */
