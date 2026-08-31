#include <iostream>
using namespace std;

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// parametri dei controllori pid
struct pid {
// parametri del controllore PID
float kp;  // guadagno proporzionale
float ki;  // guadagno integrale
float kd;  // guadagno derivativo
float kw;  // guadagno windup

// componenti del controllore PID
float proportional; // componente proporzionale
float integral;     // componente integrale
float derivative;   // componente derivativa
float output;       // output puro del PID non limitato

// variabili e accumulatori intermedi per calcolo PID
float deriv_c1; // primo coefficiente per calcolo derivata
float deriv_c2; // secondo coefficiente per calcolo derivata
float sum1;     // somma degli errori tra setpoint e valore misurato
float sum2;     // somma dei valori misurati
float sum3;     // somma dei prodotti tra indice e valore misurato

// numero di misurazioni
uint16_t data_count; // misurazioni ricevute ed elaborate
uint16_t expected_data_count; // misurazioni attese

// flag per metodo anti-windup da utilizzare
uint8_t anti_windup;
};

// costanti
#define TEMP_CTRL 2
#define TEMP_PID_KP 1
#define TEMP_PID_KI 0.1
#define TEMP_PID_KD 1
#define TEMP_PID_KW 0.3
#define TEMP_PID_WINDUP 2
#define CTRL_MIN_OUTPUT 0
#define CTRL_MAX_OUTPUT 100
#define CTRL_PWM_PERIOD 8000
#define SHT20_READ_PERIOD 2000

// struct pid
struct pid pid;

// elabora nuova misurazione
void pid_add_data(struct pid &pid, const int16_t &error, const uint16_t &measure) {
	pid.sum1 += error;
	pid.sum2 += measure;
	pid.sum3 += pid.data_count * measure;
	pid.data_count++;
	cout << " - update - err: " << error << " - meas: " << measure << endl;
}

// aggiorna output del pid
float pid_update_output(struct pid &pid) {
	// verifica assenza di dati da elaborare
	if (pid.data_count == 0) {
		pid.sum1 = 0L;
		pid.sum2 = 0UL;
		pid.sum3 = 0UL;
		return CTRL_MIN_OUTPUT;
	}

	// calcolo della componente proporzionale
	float mean_error = (float)pid.sum1 / pid.data_count;
	pid.proportional = pid.kp * mean_error;

	// calcolo del contributo della componente integrale
	float integral_contrib = pid.ki * mean_error * pid.expected_data_count * (SHT20_READ_PERIOD / 1000.0f);

	// calcolo della componente derivativa
	if (pid.data_count == pid.expected_data_count) {
		pid.derivative = pid.kd * (pid.deriv_c1 * pid.sum2 + pid.deriv_c2 * pid.sum3);
	} else if (pid.data_count == 1) {
		pid.derivative = 0.0f;
	} else {
		float c1 = -6.0f / pid.data_count / (pid.data_count + 1.0f) / (SHT20_READ_PERIOD / 1000.0f);
		float c2 = 12.0f / pid.data_count / (pid.data_count * pid.data_count - 1.0f) / (SHT20_READ_PERIOD / 1000.0f);
		pid.derivative = pid.kd * (c1 * pid.sum2 + c2 * pid.sum3);
	}

	// calcolo dell'output temporaneo del PID
	pid.output = pid.proportional + pid.integral + integral_contrib - pid.derivative;
	cout << " - output - P: " << pid.proportional << " - I: " << pid.integral + integral_contrib << " - D: " << pid.derivative << " - O: " << pid.output << endl;

	// anti-windup della componente integrale del PID
	switch (pid.anti_windup) {
		case 0: // no anti-windup
			pid.integral += integral_contrib;
			break;

		case 1: // clamping
			if (!(pid.output > CTRL_MAX_OUTPUT && mean_error > 0.0f) && !(pid.output < CTRL_MIN_OUTPUT && mean_error < 0.0f))
				pid.integral += integral_contrib;
			break;

		case 2: // back calculation
			pid.integral += integral_contrib + pid.kw * (constrain(pid.output, CTRL_MIN_OUTPUT, CTRL_MAX_OUTPUT) - pid.output);
			break;
	}

	// calcolo dell'output finale del PID
	pid.output = pid.proportional + pid.integral - pid.derivative;
	cout << " - output - P: " << pid.proportional << " - I: " << pid.integral << " - D: " << pid.derivative << " - O: " << pid.output << endl;


	// azzeramento delle variabili per il prossimo ciclo del PID
	pid.sum1 = 0L;
	pid.sum2 = 0UL;
	pid.sum3 = 0UL;
	pid.data_count = 0;

	// limitazione dell'output del PID tra CTRL_MIN_OUTPUT e CTRL_MAX_OUTPUT
	return constrain(pid.output, CTRL_MIN_OUTPUT, CTRL_MAX_OUTPUT);
}

int main() {
	// inizializzazione struct
	pid.kp = TEMP_PID_KP;
	pid.ki = TEMP_PID_KI;
	pid.kd = TEMP_PID_KD;
	pid.kw = TEMP_PID_KW * (CTRL_PWM_PERIOD / 1000.0f);
	pid.proportional = 0.0f;
	pid.integral = 0.0f;
	pid.derivative = 0.0f;
	pid.output = 0.0f;
	pid.sum1 = 0L;
	pid.sum2 = 0UL;
	pid.sum3 = 0UL;
	pid.data_count = 0;
	pid.expected_data_count = CTRL_PWM_PERIOD / SHT20_READ_PERIOD;
	pid.deriv_c1 = -6.0f / pid.expected_data_count / (pid.expected_data_count + 1) / (SHT20_READ_PERIOD / 1000.0f);
	pid.deriv_c2 = 12.0f / pid.expected_data_count / (pid.expected_data_count * pid.expected_data_count - 1) / (SHT20_READ_PERIOD / 1000.0f);
	pid.anti_windup = TEMP_PID_WINDUP;

	// log di start
	cout << " --- PID START --- " << endl;

	// ciclo di test con dati finti
	for(int i = 0; i <= 80; i += SHT20_READ_PERIOD / 1000) {
		// se si arriva all'inizio del duty cycle
		if (i % (CTRL_PWM_PERIOD / 1000) == 0) {
			// stampa log di debug
			cout << "err: " << 160 - i << " - i: " << i << endl;

			// aggiornamento output del pid
			float output = pid_update_output(pid);
		}

		// simula un errore per i = 148
		if (i == 18)
			continue;

		// aggiunta nuovo dato
		pid_add_data(pid, 160 - i, i);
	}
}
