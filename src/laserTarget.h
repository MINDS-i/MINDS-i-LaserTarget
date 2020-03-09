#pragma once
#ifndef LASERTARGET_H
#define LASERTARGET_H
#ifndef CALIBRATE_H
#include "calibrate.h"
#endif
#ifndef INTERSTATE_H
#include "interState.h"
#endif

class laserTarget {
public:
	laserTarget();
	~laserTarget();
	bool Init(
		unsigned long timer,
		int sensorValues[],
		int potValue,
		int hyperSensitveThreshold = 700, /*not used right now. above this threshold assumes the detector is flooded and just looks for pulses*/
		unsigned long calib_drag_window=500,
		unsigned long calib_sample_dt=50,
		float    calib_weight=0.5f,
		int maxSensorValue = 9999,
		int minSensorValue = 0,
		int noise = 5,
		float    min_targetHit_dt = 20.f,/* for debug raised from 20.f,*/
		unsigned long noSignalMaxGap_dt = 500,/*min effective pulse on target will be either 262ms or 788 ms +*/
		unsigned long noRiseMaxCalGap_dt = 2000,
		unsigned long sample_win_dt=5, 
		unsigned long signalPulse_dt = 262,
		unsigned long signalPulse_dt_err = 26,
		int Dh = 5,
		int nDh = -2,
		int hyper_Dh = 2,
		int hyper_nDh = -2,
		int peak_Dh = 5,
		float potOffset = 23,
		float potSF     = 3
	);
	void reset(unsigned long timer, int sensorValues[], int potValue);
  inline void ForceHyper(){ m_state->ForceHyper(); }
	bool Exec(unsigned long timer, int sensorValues[]);
	inline bool GetHyper() { return m_state->isHyper(); }
	inline bool GetPulseHigh() { return (!m_calibrate) ? m_state->GetPulseHigh() : false; }
	inline bool GetSignalPulse(){ return m_state->GetSignal();}
	inline bool GetTargetHit() { return m_state->isTargetHit(); }
	bool GetDarkValues(int values[]){ return m_darkCal->GetValues(values); }
    bool GetSignalDragValues(int values[]){return m_state->GetValues(values);}
	inline bool GetDoCalibrate() { return m_calibrate; }
	inline bool GetStateInit() { return m_stateInit; }
	inline unsigned long GetTargetHit_dt(){return m_targetHit_dt;}
	/*debug*/
  bool GetDeltaRises(int values[]);
  bool GetSensorValues(int values[]);
	inline bool doPrint() { return m_doPrint; }
  inline void GetDragValuesForI(int i, int values[]){m_state->GetDragValuesForI(i, values); return;}
  inline int GetDragIndex(){return m_state->GetDragIndex();}
  inline void GetDeltaRiseValues(int values[]){m_state->GetDeltaRiseValues(values); return;}
  inline bool isSignalDragHigh(){return m_state->isSignalDragHigh();}
  inline void GetSignalRiseDragStartValues(int values[]){ m_state->GetSignalRiseDragStartValues(values); return;}
  inline bool GetSignalDragSignal(){return m_state->GetSignalDragSignal();}
  inline void GetSignalDragTimes(int values[]){m_state->GetSignalDragTimes(values); return;}
  inline void GetSignalHighs(bool values[]){m_state->GetSignalHighs(values); return;}
	/*******/
private:
	bool m_stateInit;

	calibrate*  m_darkCal;
	interState* m_state;

	float    m_min_targetHit_dt;
	float    m_potOffset;
	float    m_potSF;

	unsigned long m_targetHit_dt;

	bool          m_calibrate;
	unsigned long m_timer;

	int      m_darkValues[NUMSENSORS];

	bool setTargetHit_dt(int potValue);

	/*** debug *****/
    int  m_deltaRises[NUMSENSORS];
    int  m_sensorValues[NUMSENSORS];
	bool m_doPrint;
	/***************/
};

#endif
