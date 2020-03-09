#pragma once
#ifndef BASE_H
#define BASE_H
#define NUMSENSORS 1
#include <math.h>

class Base {
public:
	Base():m_maxSensorValue(0), m_minSensorValue(0) { ; }
	~Base() { ; }

protected:
	int m_maxSensorValue;
	int m_minSensorValue;
};
#endif
