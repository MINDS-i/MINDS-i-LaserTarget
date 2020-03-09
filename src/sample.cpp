#include "sample.h"

sample::sample() :m_win_start_time(0), m_num_values_read(0), m_win_span(0)
{
	;
}
sample::~sample() {
	;
}
bool sample::Init(unsigned long timer, unsigned long win_span) {
	m_win_span = win_span;
	reset(timer);
}
void sample::reset(unsigned long timer) {
	m_win_start_time = timer;
	m_num_values_read = 0;
	for (int i = 0; i < NUMSENSORS; i++) {
		m_culmSumValues[i] = 0.f;
	}
}
bool sample::Exec(unsigned long timer, int sensorValues[]) {
	unsigned long dtime = timer - m_win_start_time;
	if (dtime >= m_win_span) {
		windowEnd(sensorValues);
		reset(timer);
		return false;
	}
 /*
	for (int i = 0; i < NUMSENSORS; i++) {
		m_culmSumValues[i] += ((float)sensorValues[i]);
	}
	m_num_values_read++;
  */
	return true;
}
bool sample::GetValues(int values[]) {
	for (int i = 0; i < NUMSENSORS; i++) {
		values[i] = m_values[i];
	}
	return true;
}
void sample::windowEnd(int sensorValues[]) {
  /*simplified version due to odd lock up for low sample time intervals*/
  for(int i=0; i<NUMSENSORS;i++){
      m_values[i]=sensorValues[i];
  }
	//if (m_num_values_read > 0) {

    /*
		float num_values_read = (float)m_num_values_read;
		for (int i = 0; i < NUMSENSORS; i++) {
			m_culmSumValues[i] /= num_values_read;
			m_values[i] = ((int)roundf(m_culmSumValues[i]));
		}
   */
	//}
}
