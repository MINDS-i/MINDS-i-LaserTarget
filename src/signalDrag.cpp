#include "signalDrag.h"
signalDrag::signalDrag():m_min_signalPulse_dt(0), m_max_signalPulse_dt(0), m_Dh(0), m_nDh(0), m_peak_Dh(0),
m_sample(nullptr),
m_sampleProbeRunning(false), m_signal_pulse(false), m_high(false), 
m_num_drag_vals(0), m_drag_i(0)
{
	;
}
signalDrag::~signalDrag()
{
	;
}
bool signalDrag::Init(
	sample* pSample,
	unsigned long timer,
	int maxSensorValue,
	int minSensorValue,
	unsigned long signalPulse_dt,
	unsigned long signalPulse_dt_err,
	int Dh,
    int nDh,
	int peak_Dh,
	int num_drag_values/*dt corresponds to sample_dt*num_drag_values */
)
{
	m_maxSensorValue = maxSensorValue;
	m_minSensorValue = minSensorValue;
	m_min_signalPulse_dt = signalPulse_dt - signalPulse_dt_err;
	m_max_signalPulse_dt = signalPulse_dt + signalPulse_dt_err;
	m_Dh = Dh;
    m_nDh=nDh;
	m_peak_Dh = peak_Dh;
	m_num_drag_vals = num_drag_values;
	m_sample = pSample;
	return true;
}
void signalDrag::Reset(unsigned long timer)
{
	m_signal_pulse = false;
	m_high = false;
	for (int i = 0; i < NUMSENSORS; i++) {
		m_highs[i] = false;
		for (int j = 0; j < SAMPLE_NUM; j++) {
			m_drag_vals[j][i] = m_maxSensorValue;
		}
		m_peaks[i] = 0;
        m_rise_times[i]=timer;
        m_deltaRise[i]=0;
        /***debug***/
        m_deltaTimes[i]=0;
        /************/
	}
	m_drag_i = 0;
	m_sample->reset(timer);
}
bool signalDrag::Exec(unsigned long timer, int sensorValues[], int darkValues[], bool& foundRise, bool& foundSignal)
{
    m_sampleProbeRunning=true;/*used by debug dump*/
	foundRise = false;
	foundSignal = false;
	m_signal_pulse = false;

	if (m_sample->Exec(timer, sensorValues)) {
		return m_sampleProbeRunning;/*still in measurement window nothing found*/
	}
    m_sampleProbeRunning=false;
	/*if at end of mearsurement*/
	m_sample->GetValues(m_sampleValues);/*first retrieve current sample measured values then*/
    m_high=false;
    for(int i=0; i<NUMSENSORS; i++){
		bool high = false;
		bool sens_riseFound = false; 
		bool sens_signalFound=false;
		sensorState(i, timer, m_sampleValues[i], darkValues[i], high, sens_riseFound, sens_signalFound);
        m_high = (m_high || high);
        foundRise = (foundRise || sens_riseFound);
        foundSignal = (foundSignal || sens_signalFound);
	}
	m_signal_pulse = foundSignal;

	AddDragValues(timer, m_sampleValues);/*always add the sample values to the drag window*/
	return m_sampleProbeRunning;
}
bool signalDrag::GetSampleValues(int values[]){
  for(int i=0; i<NUMSENSORS; i++){
    values[i]=m_sampleValues[i];
  }
}
void signalDrag::GetDragValuesForI(int i, int values[]){
  for(int j=0; j<SIGNAL_DEF_DRAGNUM; j++){
    values[j]= m_drag_vals[j][i]; 
  }
}
void signalDrag::GetDeltaRiseValues(int values[])
{
  for(int i=0; i<NUMSENSORS; i++){
    values[i]=m_deltaRise[i];
  }
}
void signalDrag::GetRiseStartValues(int values[])
{
  for(int i=0; i<NUMSENSORS; i++){
    values[i]=m_rise_start_vals[i];
  }
}
void signalDrag::GetDeltaTimes(int values[]){
  for(int i=0; i<NUMSENSORS; i++){
    values[i]=m_deltaTimes[i];
  }
}
void signalDrag::GetHighs(bool values[]) {
	for (int i = 0; i < NUMSENSORS; i++) {
		values[i] = m_highs[i];
	}
}
bool signalDrag::sensorState(int index, unsigned long timer, int sensorValue, int darkValue, bool& high, bool& foundRise, bool& foundSignal)
{
  foundRise=false;
  foundSignal=false;
  high = m_highs[index];
  setDeltaRise(sensorValue, index);
  if (m_peaks[index] < sensorValue)
	  m_peaks[index] = sensorValue;
  if(!high){
    high = isRise(index); 
    if (high) {
		/*this is a candidate for a pulse*/
	    /*check if the value in the interval peaked high enough above the darkvalue*/
		int peak_dh = m_peaks[index] - darkValue;
		if (peak_dh >= m_peak_Dh) {
			/*the peak was high enough so now check the time interval*/
			unsigned long dtime = timer - m_rise_times[index];
			/**debug***/
			m_deltaTimes[index] = dtime;
			/**********/
			foundSignal = isSignalInterval(dtime);/*if pulse width(rise vs drop) in time is correct then this is signal*/
		}
		/*reset the current peak value for this channel*/
		m_peaks[index] = sensorValue;
        /*after checking if was proper distance from last rise reset last rise time*/
        m_rise_times[index] = m_drag_ts[m_drag_i];
        m_last_rise_time = m_rise_times[index];
        foundRise = true;
    }
  }else{
    high = !(isDip(index));
  }
  m_highs[index] = high;
  return true;
}
void signalDrag::AddDragValues(unsigned long timer, int sensorValues[]) {
	for (int i = 0; i < NUMSENSORS; i++) {
		m_drag_vals[m_drag_i][i] = sensorValues[i]; 
	}
	m_drag_ts[m_drag_i] = timer;
	int new_i = m_drag_i + 1;
	m_drag_i = indexToDragIndex(new_i);
}
int signalDrag::indexToDragIndex(int i) {
	if (i >= m_num_drag_vals)
		i -= m_num_drag_vals;
	if (i < 0)
		i += m_num_drag_vals;
	return i;
}
void signalDrag::setDeltaRise(int sensorValue, int sensor_i){
	int tail_value = m_drag_vals[m_drag_i][sensor_i];
    m_deltaRise[sensor_i]=sensorValue-tail_value;
}
bool signalDrag::isRise(int sensor_i){
	return (m_deltaRise[sensor_i] >= m_Dh);
}
bool signalDrag::isDip(int sensor_i){
	return (m_deltaRise[sensor_i]<=m_nDh);
}
bool signalDrag::isSignalInterval(unsigned long dtime) {
	return(( dtime >= m_min_signalPulse_dt) && (dtime <= m_max_signalPulse_dt));
}
