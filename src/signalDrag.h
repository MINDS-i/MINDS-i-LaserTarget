#pragma once
#ifndef SIGNALDRAG_H
#define SIGNALDRAG_H
#ifndef SAMPLE_H
#include "sample.h"
#endif
#define SAMPLE_NUM 10 /*should be greater than SIGNAL_DEF_DRAGNUM*/
#define SIGNAL_DEF_DRAGNUM 6
class signalDrag : public Base{
public:
	signalDrag();
	~signalDrag();
	bool Init(
		sample* pSample,
		unsigned long timer,
		int maxSensorValue = 9999,
		int minSensorValue = 0,
		unsigned long signalPulse_dt=262,
		unsigned long signalPulse_dt_err=20,
		int Dh=2,
        int nDh=-1,
		int peak_Dh=-900,/*signal must reach at least this above dark to be considered candidate for good signal pulse*/
		int num_drag_values=SIGNAL_DEF_DRAGNUM/*dt corresponds to sample_dt*num_drag_values */
		/*unsigned long sample_dt=5  width of sample window is set by interState which owns pSample*/
	);
	void Reset(unsigned long timer);
	bool Exec(unsigned long timer, int sensorValues[], int darkValues[], bool& foundRise, bool& foundSignal);/*returns true if some kind of rise is found at the time it is called, false otherwise*/
	inline bool isSignalPulse() { return m_signal_pulse; }
	inline bool isHigh() { return m_high; }
	bool GetSampleValues(int values[]); 
	inline void SetDh(int Dh) { m_Dh = Dh; }
	inline void SetnDh(int nDh) { m_nDh = nDh; }
	inline void SetPeakDh(int peakDh) { m_peak_Dh = peakDh; }
	inline int GetDh() { return m_Dh; }
	inline int GetnDh() { return m_nDh; }
	inline int GetPeakDh() { return m_peak_Dh; }
  unsigned long GetLastRiseTime(){return m_last_rise_time;}
  /***debug***/
  inline bool GetIsSampleProbeRunnning(){return m_sampleProbeRunning;}
  int GetDragIndex(){return m_drag_i;}
  void GetDragValuesForI(int i, int values[]);
  void GetDeltaRiseValues(int values[]);
  void GetRiseStartValues(int values[]);
  void GetDeltaTimes(int values[]);
  void GetHighs(bool values[]);
  /***********/
private:
    unsigned long m_min_signalPulse_dt;
    unsigned long m_max_signalPulse_dt;
  	/*m_Dt; determined by samples_dt*num_drag_vals */
	int m_Dh;
    int m_nDh;/*neg Dh*/
	int m_peak_Dh;/*how high peak must at least be above darkValues to trigger as a peak*/

	sample* m_sample;/*not owned*/
    int m_sampleValues[NUMSENSORS];

	bool m_sampleProbeRunning;
	bool m_signal_pulse;
	bool m_high;
	bool m_highs[NUMSENSORS];

	int           m_peaks[NUMSENSORS];
  unsigned long m_last_rise_time;
  unsigned long m_rise_times[NUMSENSORS];
	int           m_rise_start_vals[NUMSENSORS];

	int m_drag_vals[SAMPLE_NUM][NUMSENSORS];
	unsigned long m_drag_ts[SAMPLE_NUM];
	int m_num_drag_vals;/*number of samples taken in the drag window*/
	int m_drag_i;/*current location of the index in the drag window*/
	int m_drag_t[NUMSENSORS];

    bool sensorState(int index, unsigned long timer, int sensorValue, int darkValue, bool& high, bool& foundRise, bool& foundSignal);

	void AddDragValues(unsigned long timer, int sensorValues[]);/*add a new value to sample window which is considered to be circular, a.k.a. 0th index moves*/
	int indexToDragIndex(int i);
    void setDeltaRise(int sensorValue, int sensor_i);
	bool isRise(int sensor_i);
    bool isDip(int sensor_i);
	bool isSignalInterval(unsigned long dtime);
  /***debug***/

  int m_deltaRise[NUMSENSORS];
  unsigned long m_deltaTimes[NUMSENSORS];
  /***********/
};
#endif
