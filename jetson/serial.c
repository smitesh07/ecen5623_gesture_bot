#include "serial.h"


int open_port(char* port_name)
{
	struct termios tio;
  	int tty_fd; /* File descriptor for the port */
	tio.c_iflag=0;
	tio.c_oflag=0;
	tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
	tio.c_lflag=0;
	tio.c_cc[VMIN]=1;
	tio.c_cc[VTIME]=5;

	tty_fd = open(port_name, O_RDWR );// | O_NOCTTY | O_NDELAY);
	if (tty_fd < 0)
	{
		perror("Unable to open ");
		return -1;
	}
	cfsetospeed(&tio,B115200);            // 115200 baud
	//cfsetispeed(&tio,B115200);            // 115200 baud

	tcsetattr(tty_fd,TCSANOW,&tio);
	//fcntl(tty_fd, F_SETFL, 0);

	return (tty_fd);
}


/*
int main()
{
	int port=open_port(ARDUINO_SERIAL);

	char c;
	while(1)
	{

		c=getchar();
		write(port,&c,1);
		//printf("%c",c);
		
	}

	//close(port);
	printf("Port Closed \n");
}

*/