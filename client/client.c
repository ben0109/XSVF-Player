#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

int serial_fd;
FILE *xsvf_file;

int setup_stream(void)
{
	struct termios tio;
        memset(&tio,0,sizeof(tio));
        tio.c_iflag=0;
        tio.c_oflag=0;
        tio.c_cflag=CS8|CREAD|CLOCAL;	// 8n1
        tio.c_lflag=0;
        tio.c_cc[VMIN]=1;
        tio.c_cc[VTIME]=5;
        cfsetospeed(&tio,B9600);	// bauds
	cfsetispeed(&tio,B9600);

	serial_fd = open("/dev/ttyUSB1", O_RDWR);
	if (serial_fd==0) {
		return 1;
	}

	tcsetattr(serial_fd,TCSANOW,&tio);
//	serial_fd = open("to_papilio", O_RDWR);
	return 0;
}

void send(char *line)
{
}

char line_buffer[0x100];

int readLine()
{
	int n;
	char c,*ptr;
	ptr = line_buffer;
	do {
		n = read(serial_fd,&c,1);
		if (n<=0) {
			return 1;
		}
//		c = fgetc(stdin);
		*ptr++ = c;
	} while (c!='\n');
	*ptr++ = 0;
	return 0;		
}

int command_plus()
{
	static unsigned int total = 0;
	char answer[0x10];
	char buffer[0x100];
	unsigned int size;
	int i;

	size = fread(buffer, 1, 0x80, xsvf_file);

	sprintf(answer, "+%c",size);
	write(serial_fd, answer, 2);

	for (i=0; i<size; i++) {
		write(serial_fd, &buffer[i], 1);
//		usleep(1000);
	}
	total += size;
	printf("*** sent %02x bytes (total %08x)\n",size,total);
	return 0;
}

void main()
{
	if (setup_stream()) {
		exit(1);
	}

	xsvf_file = fopen("cram.xsvf","rb");

	char *command = "~";
	write(serial_fd, command, strlen(command));

	while (1) {
		char c;
		read(serial_fd, &c, 1);
//		printf("got [%s]\n",line_buffer);
		switch (c) {
		case '+':
			command_plus();
			break;
		case 'd': readLine(); printf("DEBUG  : %s",line_buffer); break;
		case 'i': readLine(); printf("INFO   : %s",line_buffer); break;
		case 'w': readLine(); printf("WARNING: %s",line_buffer); break;
		case 'e': readLine(); printf("ERROR  : %s",line_buffer); break;
		case 'f': printf("process failed\n"); return;
		case 's': printf("process succeeded\n"); return;
		}
	}
}
