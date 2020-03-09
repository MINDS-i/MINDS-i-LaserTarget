#include "laserTarget.h"

laserTarget::laserTarget() :m_stateInit(false), m_darkCal(nullptr), m_state(nullptr),
m_min_targetHit_dt(0.f), m_potOffset(0.f), m_potSF(0.f),
m_targetHit_dt(0),
m_calibrate(false)
{
	  m_darkCal = new calibrate;
    m_state = new interState;
}
laserTarget::~laserTarget()
{
	if(m_darkCal!=nullptr){
		delete m_darkCal;
	}
	if(m_state!=nullptr){
		delete m_state;
	}
}

bool laserTarget::Init(
	unsigned long timer,
	int sensorValues[],
	int potValue,
	int hyperSensitiveThreshold,
	unsigned long calib_drag_window,
	unsigned long calib_sample_dt,
	float    calib_weight,
	int maxSensorValue,
	int minSensorValue,
	int noise,
	float min_target_hit_dt,
	unsigned long noSignalMaxGap_dt,
	unsigned long noRiseMaxCalGap_dt,
	unsigned long sample_win_dt,
	unsigned long signalPulse_dt,
	unsigned long signalPulse_dt_err,
	int Dh,
	int nDh,
	int hyper_Dh,
	int hyper_nDh,
	int peak_Dh,
	float potOffset,
	float potSF 
)
{
	m_min_targetHit_dt = min_target_hit_dt;
	m_potOffset = potOffset;
	m_potSF = potSF;
	m_darkCal->Init(timer, sensorValues, calib_drag_window, calib_sample_dt, calib_weight);
	reset(timer, sensorValues, potValue);/*needs to be run after calibrate is initialized sets m_targetHit_dt*/
	m_state->Init(timer, m_targetHit_dt, maxSensorValue, minSensorValue, noise, noSignalMaxGap_dt, noRiseMaxCalGap_dt, sample_win_dt, signalPulse_dt, signalPulse_dt_err, Dh, nDh, hyper_Dh, hyper_nDh, peak_Dh);
	m_stateInit = true;
	return true;
}
void laserTarget::reset(unsigned long timer, int sensorValues[], int potValue)
{
	setTargetHit_dt(potValue);
	m_darkCal->start(timer, sensorValues);
    m_darkCal->GetValues(m_darkValues);
    m_state->reset(timer,m_targetHit_dt);
	m_calibrate = true;
	m_timer = timer;
  /*debug*/
  m_doPrint=true;
  for(int i=0; i<NUMSENSORS; i++){
    m_deltaRises[i]=0;
  }
  /******/
}

bool laserTarget::Exec(unsigned long timer, int sensorValues[]) {
	/***debug****/
	m_doPrint = false;
	/************/
	bool stillRunning = true;
	m_darkCal->GetValues(m_darkValues);
    bool notEl = !(m_state->Exec(timer,sensorValues, m_darkValues));
	if (notEl) {
      if(m_calibrate){
		  /*was in calibration never went high still in calibration*/
		  m_darkCal->Exec(timer, sensorValues);
      }else{
        /*was not in calibration, fell down from elevated. Either the state dropped back due to no signal/rises or
         * the target was hit
         */
         if (m_state->isTargetHit()){ 
           return false;/*end of run target hit*/
	     } else {
			 /*droped out since high value conditions no longer met(no rises),
		     may need to recalibate*/
		     if (m_state->isResetCalibrate()) {
				 /*if fell out because values went too high for the calibration but no pulses are being detected
	             assume need to recalibrate at higher values*/
		         int sampleValues[NUMSENSORS];
	             m_state->GetValues(sampleValues);/*get the last sampled sensor values from m_state*/
	             m_darkCal->start(timer, sampleValues);/*restart the calibration with the higher sensor values*/
		     }
		     else {/*if fell out of el because the sensorvalues went down again, just return to calibrating, reseting the window on the calibration*/
		         m_darkCal->reset(timer);
		     }
		     m_calibrate = true;
		 }
      }/*end of if m_calibrate*/
	}else{/*end of it notEl*/
      m_calibrate=false;/*if elevated then stop callibrating*/
	}
    /****debug*****/
    if(!m_state->isStillRunningSampleProbe()){
      m_doPrint=true;
    }
	return m_doPrint;
    /**************/
	return stillRunning;
}

bool laserTarget::setTargetHit_dt(int potValue)
{
	float shifted_val = ((float)potValue) - m_potOffset;
	float targetHit_dt = shifted_val * m_potSF;
	if (targetHit_dt < m_min_targetHit_dt)
		targetHit_dt = m_min_targetHit_dt;
	m_targetHit_dt = (unsigned long)roundf(targetHit_dt);
	return true;
}
bool laserTarget::GetSensorValues(int values[]){
  for(int i=0; i<NUMSENSORS; i++){
    values[i]=m_sensorValues[i];
  }
  return true;
}
bool laserTarget::GetDeltaRises(int values[])
{
  for(int i=0; i<NUMSENSORS; i++){
    values[i]=m_deltaRises[i];
  }
  return true;
}
