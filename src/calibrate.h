#pragma once

#ifndef CALIBRATE_H
#define CALIBRATE_H
#ifndef BASE_H
#include "base.h"
#endif

class calibrate {
public:
	calibrate();
	~calibrate();
	bool Init(
		unsigned long timer,
		int      sensorValues[],
		unsigned long calib_drag_window=500,
		unsigned long calib_sample_dt=50,
		float    calib_weight=0.5f
	);
	void start(unsigned long timer, int sensorValues[]);
	void reset(unsigned long timer);
	bool Exec(unsigned long timer, int sensorValues[]);
	bool GetValues(int values[]);
private:
	unsigned long m_win_start_time;/*start time of last window*/

	unsigned long m_calib_drag_window;/*how big the window is that is used to create a new ave calib value*/
	unsigned long m_calib_sample_dt;/*interval between samples taken, the  number of samples taken in the drag window is m_calib_drag_window/m_calib_sample_dt*/
	float    m_calib_weight;/*the weight of the current drag window vs the previous calib value*/

	int m_darkValues[NUMSENSORS];
	int m_num_values_read;
	float m_culmSumValues[NUMSENSORS];
	/*helper to run*/
	bool windowEnd();
};
#endif
