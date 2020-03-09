#pragma once
#ifndef SAMPLE_H
#define SAMPLE_H
#ifndef BASE_H
#include "base.h"
#endif
class sample {
public:
	sample();
	~sample();
	bool Init(
		unsigned long timer,
		unsigned long win_span=5
	);

	void reset(unsigned long timer);
	bool Exec(unsigned long timer, int sensorValues[]);

	inline const int* GetValues() { return m_values; }
	bool GetValues(int values[]);
  unsigned long GetWinSpan(){return m_win_span;}
  unsigned long GetWinStartT(){return m_win_start_time;}
private:
	unsigned long m_win_start_time;/*start time of last sample window*/
	unsigned long m_num_values_read;/*number of samples taken*/
	float    m_culmSumValues[NUMSENSORS];
	int      m_values[NUMSENSORS];
	
	unsigned long m_win_span;/*span in time of sample window*/

	void windowEnd(int sensorValues[]);
};
#endif
