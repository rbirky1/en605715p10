/**
 * Adapted from: https://www.zaphoyd.com/websocketpp/manual/common-patterns/server-initiated-messages
 * Adapted by: Rachael Birky
 * Date: 04/19/2019
 * Intall: sudo apt-get install libboost-all-dev
 * Compile: g++ -std=c++11 i2c-master.cpp -o i2c-master -lboost_system -lpthread
 */

// Standard
#include <functional>
#include <mutex>
#include <set>
#include <thread>

// I2C
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// Webserver
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#define BYTES 40

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;

class ws_server {
public:
	ws_server(int device) {
		m_device = device;
		m_server.init_asio();
		m_server.set_open_handler(bind(&ws_server::on_open,this,std::placeholders::_1));
		m_server.set_close_handler(bind(&ws_server::on_close,this,std::placeholders::_1));
	}

	void on_open(connection_hdl hdl) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_connections.insert(hdl);
	}

	void on_close(connection_hdl hdl) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_connections.erase(hdl);
	}

	void read_device() {
		while (1) {
			// Every second
			sleep(1);

			unsigned char buffer[BYTES] = {0};
			printf("%d\n", read(m_device, buffer, BYTES));
/*
			if (read(m_device, buffer, BYTES) != BYTES) {
				printf("Failed to read from the i2c bus.\n");
			} else {
				printf("Data read: %s\n", buffer);
				/*
				// Send data to all clients
				std::string message(buffer);
				std::lock_guard<std::mutex> lock(m_mutex);
				for (auto it : m_connections) {
					m_server.send(it,message,websocketpp::frame::opcode::text);
				}
			}
*/
		}
	}

	void run(uint16_t port) {
		m_server.listen(port);
		m_server.start_accept();
		m_server.run();
	}

private:
	typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

	int m_device;
	server m_server;
	con_list m_connections;
	std::mutex m_mutex;
};

int main(int argc, char** argv) {
	char *i2c_filename;
	int i2c_fd;
	int i2c_addr;
	
	if (argc == 3) {
		i2c_filename = (char*) argv[0];
		i2c_addr = atoi(argv[1]);
		printf("Using user-defined I2C bus values.\nFile: %s\nAddress: %d\n", i2c_filename, i2c_addr);
	} else {
		i2c_filename = (char *) "/dev/i2c-1";
		i2c_addr = 0x7f;
		printf("Using default I2C bus values.\nFile: %s\nAddress: %d\n", i2c_filename, i2c_addr);
	}

	if ((i2c_fd = open(i2c_filename, O_RDWR)) < 0) {
		printf("Failed to open the i2c bus: %d\n.", i2c_fd);
		return i2c_fd;
	}

	if (ioctl(i2c_fd, I2C_SLAVE, i2c_addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		return -1;
	}

	ws_server server(i2c_fd);
	std::thread t(std::bind(&ws_server::read_device,&server));
	server.run(9002);
}
