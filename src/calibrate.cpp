#include "calibrate.h"

calibrate::calibrate() :m_win_start_time(0), m_calib_drag_window(0), m_calib_sample_dt(0), m_calib_weight(0.f), m_num_values_read(0) {
	;
}
calibrate::~calibrate()
{
	;
}
bool calibrate::Init(unsigned long timer, int sensorValues[], unsigned long calib_drag_window, unsigned long calib_sample_dt, float calib_weight) 
{
	m_calib_drag_window = calib_drag_window;
	m_calib_sample_dt = calib_sample_dt;
	m_calib_weight = calib_weight;
	start(timer, sensorValues);
	return true;
}
void calibrate::start(unsigned long timer, int sensorValues[])
{
	reset(timer);
	for (int i = 0; i < NUMSENSORS; i++) {
		m_darkValues[i] = sensorValues[i];
	}
}
void calibrate::reset(unsigned long timer) {
	m_win_start_time = timer;
	m_num_values_read = 0;
	for (int i = 0; i < NUMSENSORS; i++) {
		m_culmSumValues[i] = 0.f;
	}
}
bool calibrate::Exec(unsigned long timer, int sensorValues[]) {
	unsigned long dtime = timer - m_win_start_time;
	if (dtime >= m_calib_drag_window) {
		windowEnd();
		reset(timer);
		return false;
	}
	unsigned long last_interval_time = ((unsigned long)m_num_values_read)*m_calib_sample_dt;
	if (last_interval_time >= dtime)
		return true;
	for (int i = 0; i < NUMSENSORS; i++) {
		m_culmSumValues[i] += ((float)sensorValues[i]);
	}
	m_num_values_read++;
	return true;
}
bool calibrate::GetValues(int values[]) {
	for (int i = 0; i < NUMSENSORS; i++) {
		values[i] = m_darkValues[i];
	}
	return true;
}
bool calibrate::windowEnd() {
	if (m_num_values_read > 0) {
		float num_values_read = (float)m_num_values_read;
		for (int i = 0; i < NUMSENSORS; i++) {
			m_culmSumValues[i] /= num_values_read;
			float darkValue = m_darkValues[i];
			darkValue = (1.f - m_calib_weight)*darkValue + m_calib_weight * m_culmSumValues[i];
			m_darkValues[i] = (int)roundf(darkValue);
		}
	}
	return true;
}
