#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>


// port name of the arduino board connected to the serial port
//#define PORT_NAME "/dev/cu.usbserial-1140"
#define PORT_NAME "/dev/cu.usbmodem11401"

// message code received from the arduino board
#define CSV_DATA 0
#define NEW_TEMP_SETPOINT 1
#define NEW_RH_SETPOINT 2
#define MAN_HEATER_ON 3
#define MAN_HEATER_OFF 4
#define MAN_HUMIDIFIER_ON 5
#define MAN_HUMIDIFIER_OFF 6
#define MAN_LIGHT_ON 7
#define MAN_LIGHT_OFF 8
#define AUTO_HEATER_ON 9
#define AUTO_HEATER_OFF 10
#define AUTO_HUMIDIFIER_ON 11
#define AUTO_HUMIDIFIER_OFF 12
#define ERROR 13
#define BOOT 14

/**
 * @brief Determines the code of a message based on its content
 *
 * @param message message received from the arduino board
 * @return int code of the message, or -1 if the message is unknown
 */
int message_code(const char *message);


int main() {
	// port name of the arduino board connected to the serial port
	const char *portname = PORT_NAME;

	// local variables for reading operations
	char buffer[256]; // buffer to store the data read from the serial port
	int buf_idx = 0; // index to keep track of the current position in the buffer
	int n; // number of bytes read from the serial port

	// structure to store the status of the incubator
	struct status {
		float temp;
		float rh;
		float set_temp;
		float set_rh;
		int heater;
		int humidifier;
	} status = {0.0, 0.0, 0.0, 0.0, 0, 0};

	// variables for timestamping
	time_t now = 0;
    struct tm *t = NULL;
	char timestamp[20];

	// line written in the csv file
	char csv_line[256];

	/**
	 * --- 1. open the serial port ---
	 *
	 * The serial port is connected to the process through a file descriptor,
	 * using the open() system call (everything is a file).
	 *
	 * The following flags are used:
	 * - O_RDWR: open for reading and writing
	 * - O_NOCTTY: do not make this port the controlling terminal for the process
	 */

	// open the serial port
	int serial_port = open(portname, O_RDWR | O_NOCTTY);

	// check if the serial port was opened successfully
	if (serial_port < 0) {
		fprintf(stderr, "Error %d opening %s: %s\n", errno, portname, strerror(errno));
		return 1;
	}

	/**
	 * --- 2. configure the serial port ---
	 *
	 * Serial port is configured using the termios structure, to set the baud
	 * rate and other parameters to match the serial communications protocol
	 * of the arduino board.
	 *
	 * The following parameters are set:
	 *
	 * Control modes:
	 *  - ~CSIZE: bit mask for data bits (clear current data bits setting)
	 *  - CS8: set 8 data bits
	 *  - ~CSTOPB: disable double stop bit, use 1 stop bit
	 *  - CREAD: enable receiver to read incoming data
	 * 	- ~PARENB: disable parity bit generation and checking
	 *  - CLOCAL: ignore modem control lines
	 *  - ~CRTSCTS: disable hardware flow control
	 *
	 * Local modes:
	 *  - ~ECHO: disable echo
	 *  - ~ISIG: disable interpretation of INTR, QUIT, SUSP
	 *  - ~ICANON: disable canonical mode (buffer data until the "\n" character)
	 *
	 * Input modes:
	 * - ~IGNBRK: enable BREAK condition
	 * - ~BRKINT: disable conversion of BREAK to SIGINTR
	 * - ~PARMRK: disable marking parity and framing errors
	 * - ~ISTRIP: disable striping the 8th bit off from every byte
	 * - ~INLCR: disable conversion of NL to CR
	 * - ~IGNCR: ignore CR (useless on unix systems)
	 * - ~ICRNL: disable conversion of CR to NL
	 * - ~IXON: disable software flow control on output
	 * - ~IXOFF: disable input flow control on input
	 * - ~IXANY: disable software flow control
	 *
	 * Output modes:
	 * - ~OPOST: disable special interpretation of output bytes
	 * - ~ONLCR: disable conversion of NL to CR or LF
	 *
	 * Baud rate speed:
	 * - 115200 baud, set through the cfsetispeed() and cfsetospeed() functions
	 */

	// get the current configuration of the serial port
	struct termios tty;
	if (tcgetattr(serial_port, &tty) != 0) {
		fprintf(stderr, "Error %d from tcgetattr: %s\n", errno, strerror(errno));
		return 1;
	}

	// set the configuration of the serial port
	// control modes
	tty.c_cflag &= ~CSIZE;  // clear current data bits setting
	tty.c_cflag |= CS8;     // 8 data bits
	tty.c_cflag &= ~PARENB; // no parity bit
	tty.c_cflag &= ~CSTOPB; // 1 stop bit
	tty.c_cflag &= ~CRTSCTS;// disable hardware flow control
	tty.c_cflag |= CREAD | CLOCAL; // enable read and ignore control lines

	// input modes
	tty.c_lflag &= ~ICANON; // disable canonical mode
	tty.c_lflag &= ~ECHO;   // disable echo
	tty.c_lflag &= ~ISIG;   // disable interpretation of INTR, QUIT, SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // disable software flow control
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

	// output modes
	tty.c_oflag &= ~OPOST; // prevent special interpretation of output bytes
	tty.c_oflag &= ~ONLCR; // prevent conversion of newline to carriage return/line feed

	// set baud rate
	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);

	// apply the configuration settings to the serial port
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
		fprintf(stderr, "Error %d from tcsetattr: %s\n", errno, strerror(errno));
		return 1;
	}

	/**
	 * --- 3. read from the serial port and write to CSV file ---
	 *
	 * Opens the CSV file in append mode, writes the header, and then writes
	 * the data read from the serial port to the CSV file every time a newline
	 * character is received.
	 */

	// open CSV File in Append Mode ("a")
	FILE *csv_file = fopen("arduino_data.csv", "a");
	if (csv_file == NULL) {
		fprintf(stderr, "Failed to open CSV file!\n");
		return 1;
	}

	// output log message
	printf("Reading from %s... Press Ctrl+C to stop.\n", portname);

	// infinite reading loop
	while (1) {
		// initialize buffer index and number of bytes read
		n = buf_idx = 0;

		// read data from the serial port until newline character is received
		while(buf_idx < sizeof(buffer) - 1) {
			// read one byte from the serial port
			n = read(serial_port, buffer + buf_idx, 1);

			// if CRLF is received, exit the loop
			if (buffer[buf_idx] == '\n' && buffer[buf_idx-1] == '\r')
				break;

			// else increment buffer index by the number of bytes read
			else
				buf_idx += n;
		}

		// adding null terminator to the end of the string
		buffer[--buf_idx] = '\0';

		// log the received line
		printf("in:  %s\n", buffer);

		// process the received line based on its message code
		switch (message_code(buffer)) {
			// new data received, write to CSV file with timestamp
			case CSV_DATA:
				// get the current time and format it as a timestamp
				now = time(NULL);
    			t = localtime(&now);
				strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

				// get data from the received line
				sscanf(buffer, "CSV-: T: %f°C | RH: %f%% | Set_T: %f°C | Set_RH: %f%%", &status.temp, &status.rh, &status.set_temp, &status.set_rh);

				// create a CSV line with the timestamp and data
				sprintf(csv_line, "%s,%.2f,%.2f,%.2f,%.2f,%d,%d\n", timestamp, status.temp, status.rh, status.set_temp, status.set_rh, status.heater, status.humidifier);

				// append data to the CSV file with timestamp and flush to disk immediately
				fprintf(csv_file, "%s", csv_line);
				fflush(csv_file);

				// log the printed line
				printf("out: %s\n", csv_line);
				break;

			// new temperature setpoint received
			case NEW_TEMP_SETPOINT:
				// get new temperature setpoint from the received line
				sscanf(buffer, "SET0: Nuovo setpoint di temperatura: %f°C", &status.set_temp);
				break;

			// new humidity setpoint received
			case NEW_RH_SETPOINT:
				// get new humidity setpoint from the received line
				sscanf(buffer, "SET1: Nuovo setpoint di umidità: %f%%", &status.set_rh);
				break;

			// manual or automatic heater on received
			case MAN_HEATER_ON:
			case AUTO_HEATER_ON:
				status.heater = 1;
				break;

			// manual or automatic heater off received
			case MAN_HEATER_OFF:
			case AUTO_HEATER_OFF:
				status.heater = 0;
				break;

			// manual or automatic humidifier on received
			case MAN_HUMIDIFIER_ON:
			case AUTO_HUMIDIFIER_ON:
				status.humidifier = 1;
				break;

			// manual or automatic humidifier off received
			case MAN_HUMIDIFIER_OFF:
			case AUTO_HUMIDIFIER_OFF:
				status.humidifier = 0;
				break;

			// manual light on/off received, but no action is needed
			case MAN_LIGHT_ON:
			case MAN_LIGHT_OFF:
				// intentionally left blank
				break;

			// error message received
			case ERROR:
				printf("Error received: %s\n", buffer);
				break;

			// boot message received, write the CSV header
			case BOOT:
				// write the CSV header and force saving to disk immediately
				fprintf(csv_file, "Timestamp,Temperature,Humidity,Setpoint_Temperature,Setpoint_Humidity,Heater,Humidifier\n");
				fflush(csv_file);
				break;

			// unknown message received
			default:
				printf("Unknown message received: %s\n", buffer);
				break;
		}
	}

	close(serial_port);
	fclose(csv_file);
	return 0;
}

// determines the code of a message based on its content
int message_code(const char *message) {
	if (strncmp(message, "CSV-:", 4) == 0)
		return CSV_DATA; // dati csv
	else if (strncmp(message, "SET0", 4) == 0)
		return NEW_TEMP_SETPOINT; // nuovo setpoint temperatura
	else if (strncmp(message, "SET1", 4) == 0)
		return NEW_RH_SETPOINT; // nuovo setpoint umidità
	else if (strncmp(message, "ACT0", 4) == 0)
		return MAN_HEATER_ON; // accensione manuale del riscaldatore
	else if (strncmp(message, "ACT1", 4) == 0)
		return MAN_HEATER_OFF; // spegnimento manuale del riscaldatore
	else if (strncmp(message, "ACT2", 4) == 0)
		return MAN_HUMIDIFIER_ON; // accensione manuale dell'umidificatore
	else if (strncmp(message, "ACT3", 4) == 0)
		return MAN_HUMIDIFIER_OFF; // spegnimento manuale dell'umidificatore
	else if (strncmp(message, "ACT4", 4) == 0)
		return MAN_LIGHT_ON; // accensione manuale della luce
	else if (strncmp(message, "ACT5", 4) == 0)
		return MAN_LIGHT_OFF; // spegnimento manuale della luce
	else if (strncmp(message, "ACT6", 4) == 0)
		return AUTO_HEATER_ON; // accensione automatica del riscaldatore
	else if (strncmp(message, "ACT7", 4) == 0)
		return AUTO_HEATER_OFF; // spegnimento automatico del riscaldatore
	else if (strncmp(message, "ACT8", 4) == 0)
		return AUTO_HUMIDIFIER_ON; // accensione automatica dell'umidificatore
	else if (strncmp(message, "ACT9", 4) == 0)
		return AUTO_HUMIDIFIER_OFF; // spegnimento automatico dell'umidificatore
	else if (strncmp(message, "ERR-", 4) == 0)
		return ERROR; // errore
	else if (strncmp(message, "BOOT", 4) == 0)
		return BOOT; // errore
	else
		return -1; // Unknown message
}
