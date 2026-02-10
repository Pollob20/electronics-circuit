#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double sensor_read(double t) {
    return 2.5 + sin(t);
}

double adc_convert(double v) {
    return (v / 5.0) * 1023.0;
}

double dac_convert(double d) {
    return (d / 1023.0) * 5.0;
}

int main() {
    double t = 0.0;
    double dt = 0.1;
    double setpoint = 3.0;
    double kp = 1.2;
    double control = 0.0;

    for(int i = 0; i < 200; i++) {
        double analog = sensor_read(t);
        double digital = adc_convert(analog);
        double measured = dac_convert(digital);
        double error = setpoint - measured;
        control = kp * error;
        printf("t=%.2f  sensor=%.2f  control=%.2f\n", t, measured, control);
        t += dt;
    }

    return 0;
}
