#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cerrno>

int main() {
	const char* portName = "/dev/cu.usbserial-1140";
	int serial_port = open(portName, O_RDWR);

	if (serial_port < 0) {
		std::cerr << "Error " << errno << " opening " << portName << ": " << strerror(errno) << std::endl;
		return 1;
	}

	// Configure Serial Port Parameters
	struct termios tty;
	if(tcgetattr(serial_port, &tty) != 0) {
		std::cerr << "Error " << errno << " from tcgetattr" << std::endl;
		return 1;
	}

	tty.c_cflag &= ~PARENB; // No parity bit
	tty.c_cflag &= ~CSTOPB; // 1 stop bit
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;     // 8 data bits
	tty.c_cflag &= ~CRTSCTS; // Disable hardware flow control
	tty.c_cflag |= CREAD | CLOCAL; // Enable read and ignore control lines

	tty.c_lflag &= ~ICANON; // Disable canonical mode (line-by-line)
	tty.c_lflag &= ~ECHO;   // Disable echo
	tty.c_lflag &= ~ISIG;   // Disable interpretation of INTR, QUIT, SUSP

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

	// Set Baud Rate (Must match Arduino Serial.begin)
	cfsetispeed(&tty, B9600);
	cfsetospeed(&tty, B9600);

	if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
		std::cerr << "Error " << errno << " from tcsetattr" << std::endl;
		return 1;
	}

	// Open CSV File
	std::ofstream csvFile("arduino_data.csv");
	if (!csvFile.is_open()) {
		std::cerr << "Failed to open CSV file!" << std::endl;
		return 1;
	}

	csvFile << "Sensor1,Sensor2\n"; // Write CSV Header

	char read_buf[256];
	std::string line = "";

	std::cout << "Reading from " << portName << "... Press Ctrl+C to stop." << std::endl;

	// Read Loop
	while (true) {
		int n = read(serial_port, &read_buf, sizeof(read_buf));
		if (n > 0) {
			for(int i = 0; i < n; i++) {
				if(read_buf[i] == '\n') {
					csvFile << line << "\n";
					csvFile.flush(); // Force save to disk immediately
					std::cout << "Logged: " << line << std::endl;
					line = "";       // Reset line buffer
				} else if (read_buf[i] != '\r') {
					line += read_buf[i]; // Append char to string (ignoring carriage returns)
				}
			}
		}
	}

	close(serial_port);
	return 0;
}
