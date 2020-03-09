#pragma once
#ifndef INTERSTATE_H
#define INTERSTATE_H
#define PULSETRAINLEN 100
#define INTERSTATE_MINPEAKDH -500
#ifndef SIGNALDRAG_H
#include "signalDrag.h"
#endif


class interState : public Base{
public:
	interState();
	~interState();
	bool Init(
		unsigned long timer,
		unsigned long targetHit_dt,
		int maxSensorValue=9999,
		int minSensorValue=0,
		int noise = 10,
		unsigned long noSignalMaxGap_dt=300,/*should always be greater than expected signal width inc err*/
		unsigned long noRiseMaxCalGap_dt = 2000,
		unsigned long sample_win_dt = 5, /*defines the time interval in ms between samples of the sensor values being taken*/
		unsigned long signalPulse_dt = 262, /*expected width between rises in laser pulse*/
		unsigned long signalPulse_dt_err = 20,/*allowed err or the measured width between rises in the pulse*/
		int Dh = 2,/*determines how steep the increase in the pulse is needed to be measured as the start of a pulse, Dh/(sample_win_dt*SIGNAL_DEF_DRAGNUM) = dRise/dt threshold for signal pulse rise detection*/
		int nDh = -1,/*determines how steep of a decent in the pulse is needed to be considered the start of the trough between signal rises or the end of a pulse; used to keep pulses from running together*/
		int hyper_Dh = 2,/*hyper sensitive values*/
		int hyper_nDh = -2,
		int peak_Dh = 10/*signal must reach at least this above dark to be considered candidate for good signal pulse*/
	);

	void reset(unsigned long timer, unsigned long targetHit_dt=0);
	bool Exec(unsigned long timer, int sensorValues[], int darkValues[]);
	void ForceHyper();

  inline bool isHyper(){ return m_hyper; }
	inline bool isElevated() { return m_elevated; }
	inline bool isTargetHit() { return m_targetHit; }
	inline bool isResetCalibrate() { return m_doResetCalibrate; }
	inline void SetResetCalibrate(bool val) { m_doResetCalibrate = val; }

	bool GetValues(int values[]){m_sample->GetValues(values); return true;}
	inline bool GetPulseHigh() { return m_pulseHigh; }
  inline bool GetSignal(){ return m_signal;}
  /**debug**/
  inline bool isStillRunningSampleProbe(){ return m_stillRunningSampleProbe;}
  inline void GetDragValuesForI(int i, int values[]){m_signalDrag->GetDragValuesForI(i, values); return;}
  inline int GetDragIndex(){return m_signalDrag->GetDragIndex();}
  inline void GetDeltaRiseValues(int values[]){m_signalDrag->GetDeltaRiseValues(values); return;}
  inline bool isSignalDragHigh(){return m_signalDrag->isHigh();}
  inline void GetSignalRiseDragStartValues(int values[]){ m_signalDrag->GetRiseStartValues(values); return;}
  inline bool GetSignalDragSignal(){return m_signalDrag->isSignalPulse();}
  inline void GetSignalDragTimes(int values[]){m_signalDrag->GetDeltaTimes(values); return;}
  void GetTriggerRises(int values[]);
  void GetSignalHighs(bool values[]) { m_signalDrag->GetHighs(values); return; }
  /*********/
private:
	sample*     m_sample;
	signalDrag* m_signalDrag;


	int m_noise;/*values must drop below noise before calibration can resume, noise  should be less than deltaRise*/
	unsigned long m_noSignalMaxGap_dt; /*maximum interval in time from last good rise that could be matched to a previous rise*/
	unsigned long m_noRiseMaxCalGap_dt;/*maximum interval with no rises detected before the sensor assumes that it is at a new high and starts recalibrating*/
	unsigned long m_targetHit_dt;/*time signal needs to be present before target is considered hit*/

	bool m_stillRunningSampleProbe;

	bool m_targetHit;
  bool m_hyper;
	bool m_doResetCalibrate;
	bool m_elevated;/*is elevated*/
	bool m_signal;

	bool m_pulseHigh;

	int           m_sample_values[NUMSENSORS];
	unsigned long m_el_start_time;/*start time for elevated*/
	unsigned long m_signal_start_time;/*start time for signal found*/
    unsigned long m_last_rise_time;
	unsigned long m_last_good_rise_time;/*last time a good rise that was within the correct interval in time of a previous rise was found*/

	bool probe_trigger(int probeValues[], int darkValues[]);

	bool inSignal(unsigned long timer);
	bool inElevated(unsigned long timer, int darkValues[]);

	int m_alt_Dh;/*used to hold the hyper value when hyper is not active*/
	int m_alt_nDh;

 /***debug *****/
  int m_triggerRises[NUMSENSORS];
 /**************/
};

#endif
