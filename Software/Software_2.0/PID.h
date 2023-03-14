#include <stdint.h>

class PID {
	public:
	PID();
	float Update(float setpoint, float measurement);

  private:
	float Kp;
	float Ki;
	float Kd;
	float tau;
	float limMin;
	float limMax;
	float limMinInt = -255;
	float limMaxInt = 255;
	float T;
	float integrator;
	float prevError;
	float differentiator;
	float prevMeasurement;
	float out;
};