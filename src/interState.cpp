#include "interState.h"
interState::interState() : m_sample(nullptr), m_signalDrag(nullptr), m_noise(0),
m_noSignalMaxGap_dt(0), m_noRiseMaxCalGap_dt(0), m_targetHit_dt(0), m_stillRunningSampleProbe(false),
m_targetHit(false), m_hyper(false), m_doResetCalibrate(false), m_elevated(false), m_signal(false), 
m_pulseHigh(false),
m_el_start_time(0), m_signal_start_time(0), m_last_rise_time(0), m_last_good_rise_time(0)
{
    m_sample = new sample;
	  m_signalDrag = new signalDrag;
}
interState::~interState()
{
	if (m_sample != nullptr) {
		delete m_sample;
	}
	if (m_signalDrag != nullptr) {
		delete m_signalDrag;
	}
}
bool interState::Init(
	unsigned long timer,
	unsigned long targetHit_dt,
	int maxSensorValue,
	int minSensorValue,
	int noise,
	unsigned long noSignalMaxGap_dt,
	unsigned long noRiseMaxCalGap_dt,
	unsigned long sample_win_dt,
	unsigned long signalPulse_dt,
	unsigned long signalPulse_dt_err,
	int Dh,
	int nDh,
	int hyper_Dh,
	int hyper_nDh,
	int peak_Dh 
)
{
	m_maxSensorValue = maxSensorValue;
	m_minSensorValue = minSensorValue;
	m_noise = noise;
	m_noSignalMaxGap_dt = noSignalMaxGap_dt;
	m_noRiseMaxCalGap_dt = noRiseMaxCalGap_dt;
	m_alt_Dh = hyper_Dh;
	m_alt_nDh = hyper_nDh;
	m_sample->Init(timer, sample_win_dt);
	m_signalDrag->Init(m_sample, timer, maxSensorValue, minSensorValue, signalPulse_dt, signalPulse_dt_err, Dh, nDh, peak_Dh);/*use default values for m_signalDrag see header file of signalDrag*/
	reset(timer, targetHit_dt);/*must be called after init of long & short samples*/
	return true;
}
void interState::reset(unsigned long timer, unsigned long targetHit_dt) {
	m_doResetCalibrate = false;
	m_sample->reset(timer);
	m_signal = false;
	m_targetHit = false;
  m_hyper=false;
	m_elevated = false;
	m_pulseHigh = false;
	m_targetHit_dt = targetHit_dt;
	m_last_rise_time = timer;
}
bool interState::Exec(unsigned long timer, int sensorValues[], int darkValues[])
{
	/*run signal search and check if state still valid*/
	if (m_elevated) {
		/*if state has gone elevated check for signal*/
		bool foundRise = false;
		bool goodSignalRise = false;
		m_stillRunningSampleProbe = m_signalDrag->Exec(timer, sensorValues, darkValues, foundRise, goodSignalRise);
		if (!m_stillRunningSampleProbe) {
			m_sample->GetValues(m_sample_values);
			m_pulseHigh = m_signalDrag->isHigh();
			/*making this extremly sensitive, if time is low enough will  trigger after one pulse, this also means that the pot must be adjusted so that 
			* the min effective trigger time is greater than m_targetHit_dt+noSignalMaxGap_dt to have an effect */
			/* check if elapsed time is long enough to consider target hit*/
			if(m_signal){
				unsigned long dtime_signal_start = timer - m_signal_start_time;
				if (dtime_signal_start >= m_targetHit_dt) {
					m_targetHit = true;
					return false;/*leave this state since success condition has been fullfilled*/
				}
			}
            /*  end of check for target hit                                */
			if (foundRise) {
				m_last_rise_time = timer;
				if (goodSignalRise) {/*flag set indicates that this rise is at the end of a good signal pulse*/
					m_last_good_rise_time = m_signalDrag->GetLastRiseTime();
					/*check if first signal rise*/
					if (!m_signal) {
						/*if first signal rise*/
						m_signal_start_time = m_signalDrag->GetLastRiseTime();
						m_signal = true;
					}
				}//end of if Good Signal Rise
			}/*if no rises found then this may just be a shift to a higher light value, check about dropping out of elevated*/
			else {
				m_elevated = inElevated(timer, darkValues);/*check if a rise(not necessarly a signal) has been found reciently enough to keep state in elevated*/
				if (!m_elevated) {
					m_doResetCalibrate = probe_trigger(m_sample_values, darkValues);/*if there is an immediate trigged jump to elevated
																				 then the background dark values have increased
																				 and the calibration needs to be restarted*/
				}
			}	/*end of found rise   the signalDrag resets its sample internally, reset is only needed when the signalDrag ends/begins again*/
			m_signal = inSignal(timer);/*check if enough time has elapsed since last good signal rise was found to drop out of signal */
		}/*end of stillRunningProbe*/
		/*if still running probe don't do anything just leave in elevated state until probe finishes*/
	}
	else {/*if not in elevated check if state should go to elevated*/
		/*run sample taker to check for fast elevations*/
		m_stillRunningSampleProbe = m_sample->Exec(timer, sensorValues);
		if (!m_stillRunningSampleProbe) {
			/*if sample taker is done*/
			/*save the values and reset the sample taker*/
			m_sample->GetValues(m_sample_values);
			/*check to see if these values have triggered an elevated state*/
			m_elevated = probe_trigger(m_sample_values, darkValues);
			if (m_elevated) {
				m_el_start_time = timer;
				m_last_rise_time = timer;
				/*start the signalDrag*/
				m_signalDrag->Reset(timer);
			}
		}
	}
	return m_elevated;
}
void interState::ForceHyper() {
	m_elevated = true;
  m_hyper = true;
	m_signalDrag->SetPeakDh(INTERSTATE_MINPEAKDH);
	m_signalDrag->SetDh(m_alt_Dh);
	m_signalDrag->SetnDh(m_alt_nDh);
}

bool interState::probe_trigger(int probeValues[], int darkValues[]) {
	for (int i = 0; i < NUMSENSORS; i++) {
		int deltaRise = probeValues[i] - darkValues[i];
		/*debug*/
		m_triggerRises[i] = deltaRise;
		/********/
		if (deltaRise > m_noise)
			return true;
	}
	return false;
}


bool interState::inSignal(unsigned long timer) 
{
	if (!m_signal)
		return false;
	unsigned long dtime = timer - m_last_good_rise_time;
	if (dtime >= m_noSignalMaxGap_dt) {
		return false;
	}
	return true;
}
bool interState::inElevated(unsigned long timer, int darkValues[]) {
  if(m_hyper)
    return m_hyper;
	bool stillHigh;
	unsigned long dtime = timer - m_last_rise_time;
	stillHigh = (dtime <= m_noRiseMaxCalGap_dt);
	return stillHigh;
}
void interState::GetTriggerRises(int values[]) {
	for (int i = 0; i < NUMSENSORS; i++) {
		values[i] = m_triggerRises[i];
	}
}
