#ifndef MOTOR_H
#define MOTOR_H

#include "driver/pcnt.h"
#define ENC_A 32
#define ENC_B 33
#define PCNT_UNIT PCNT_UNIT_0
#define COUNTS_PER_REVOLUTION 480.0f // Example value, adjust as necessary
#define POS_PIN 19
#define NEG_PIN 18
#define PWM_PIN 5

class Motor {
    private:
        // PID control parameters and state variables
        float RPM;                  // Current revolutions per minute
        float setpoint;             // Desired RPM
        float error;                // Difference between setpoint and actual RPM
        float Kp;                   // Proportional gain
        float Ki;                   // Integral gain
        float controlSignal;        // Control signal to the motor

        // Private methods
        void setRPM(unsigned long last_report);
        void setError();
    public:

        // Constructor to initialize PID parameters and state variables
        Motor(float kp, float ki);

        // Setter methods

        // Method to compute the control signal using PID control
        void computeControlSignal();
        // Method to set a new RPM setpoint
        void setSetpoint(float sp);

        // Method to set rpm directly (for testing purposes)
        void setRPMDirect(float rpm);

        // Method to clear the integral term (if needed)
        void clearIntegral();

        // Getter methods

        // Method to get the current RPM
        float getRPM(unsigned long last_report);
        // Method to get the current control signal
        float getControlSignal();
        // Method to get the current error
        float getError();
        // Method to get the current setpoint
        float getSetpoint();
};

#endif // MOTOR_H