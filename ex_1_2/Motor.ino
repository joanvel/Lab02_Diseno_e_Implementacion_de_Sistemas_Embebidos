#include "Motor.h"

volatile static float integral = 0.0f; // Moved integral to file scope to retain its value across calls

Motor::Motor(float kp, float ki)
    : RPM(0.0f), setpoint(0.0f), error(0.0f), Kp(kp), Ki(ki), controlSignal(0) {}

void Motor::setRPM(unsigned long last_report) {
    // Assuming counts is the number of encoder counts in the given time interval (in seconds)
    // and that we know the counts per revolution (CPR) of the motor encoder.
    int16_t counts;
    pcnt_get_counter_value(PCNT_UNIT, &counts);
    unsigned long current_time = millis();
    float timeInterval = (current_time - last_report) / 1000.0f; // Convert ms to seconds
    RPM = (counts / (COUNTS_PER_REVOLUTION*2)) * (60.0f / timeInterval);
    pcnt_counter_clear(PCNT_UNIT); // Clear counter after reading
}

void Motor::setError() {
    error = setpoint - RPM;
}

void Motor::computeControlSignal() {
    setError();
    // Simple PI control
    float P = Kp * error;
    integral += error; // Accumulate the integral
    float I = Ki * integral;

    float output = P + I;
    controlSignal = output;

    // Clamp control signal to valid range (e.g., 0 to 255 for an 8-bit PWM)
    if(output < 0 ) {
        output = -output;
        digitalWrite(POS_PIN, LOW);
        digitalWrite(NEG_PIN, HIGH);
    } else {
        digitalWrite(POS_PIN, HIGH);
        digitalWrite(NEG_PIN, LOW);
    }
    if (output > 255) {
        output = 255;
    }
    analogWrite(PWM_PIN, static_cast<unsigned int>(output));
}

void Motor::setSetpoint(float sp) {
    setpoint = sp;
}

void Motor::setRPMDirect(float rpm) {
    if (rpm < 0) {
        rpm = -rpm;
        digitalWrite(POS_PIN, LOW);
        digitalWrite(NEG_PIN, HIGH);
    } else {
        digitalWrite(POS_PIN, HIGH);
        digitalWrite(NEG_PIN, LOW);
    }
    RPM = rpm;
    analogWrite(PWM_PIN, static_cast<unsigned int>(rpm / 150 * 255)); // Assuming rpm max is 150 for testing purposes
}

float Motor::getRPM(unsigned long last_report) {
    setRPM(last_report);
    return RPM;
}

float Motor::getControlSignal() {
    return controlSignal;
}

float Motor::getError() {
    setError();
    return error;
}

float Motor::getSetpoint() {
    return setpoint;
}

void Motor::clearIntegral() {
    // Clear the integral term (if needed)
    // This function can be expanded if integral is made a member variable
    integral = 0.0f;
}