#include <functional>
#include <mutex>
#include <set>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

using namespace std;

int debug = 0;

int main() {
	int file_i2c;
	int length;
	int addr = 0x7f;
	signed char data[6];

	while (true) {
		char *filename = (char*)"/dev/i2c-1";
		if ((file_i2c = open(filename, O_RDWR)) < 0)
		{
			printf("Failed to open the i2c bus");
			return errno;
		}

		if (ioctl(file_i2c, I2C_SLAVE, addr) < 0)
		{
			printf("Failed to acquire bus access and/or talk to slave.\n");
			return errno;
		}

		length = 6;
		if (read(file_i2c, data, length) != length) {
			printf("Failed to read from the i2c bus.\n");
		} else {
			if (debug) {
				cout << (int)data[0] << endl;
				cout << (int)data[1] << endl;
				cout << (int)data[2] << endl;
				cout << (int)data[3] << endl;
				cout << (int)data[4] << endl;
				cout << (int)data[5] << endl;
			}
			int roll_i = ((data[1] << 8) | (data[0]));
			int pitch_i = ((data[3] << 8) | (data[2]));
			int yaw_i = ((data[5] << 8) | (data[4]));
			cout << "roll: " << roll_i << ", pitch:" << pitch_i << ", yaw: " << yaw_i << endl;
		}
		sleep(3);
	}
	return 0;
}
