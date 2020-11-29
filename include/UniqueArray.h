#pragma once
#include "Countable.h"

template <typename T, int rawSize>
class UniqueArray : public Countable<T>
{
public:
	UniqueArray();
	void flush();
	bool add(T element);
	unsigned int count() const override
	{
		return numberOfElements;
	}

	bool full() const
	{
		return (count() >= rawSize);
	}

	const T& operator[](unsigned int index) const override
	{
		return raw[index]; /* unsafe */
	}

	T& operator[](unsigned int index) override
	{
		return raw[index]; /* unsafe */
	}
private:
	volatile int numberOfElements;
	T raw[rawSize];
};
template <typename T, int rawSize> bool UniqueArray<T, rawSize>::add(T element)
{
	if(full()) {
		return false;
	}

	for(size_t i{0}; i<numberOfElements; ++i)
	{
		if( raw[i] == element ) return false;
	}

	raw[numberOfElements++] = element;
	return true;
}
template <typename T, int rawSize>
void UniqueArray<T, rawSize>::flush()
{
	numberOfElements = 0;
	raw[0]=T{};
}
template <typename T, int rawSize>
UniqueArray<T, rawSize>::UniqueArray()
{
	flush();
}
