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

	if (sscanf(line_buffer,"+%02x\n",&size)<1) {
		return 1;
	}

	size = fread(buffer, 1, size, xsvf_file);

	sprintf(answer, "+%02x\n",size);
	write(serial_fd, answer, strlen(answer));

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

	char *command = "S\n";
	write(serial_fd, command, strlen(command));

	while (1) {
		if (readLine()) {
			fprintf(stderr,"error while reading data\n");
		}
//		printf("got [%s]\n",line_buffer);
		switch (line_buffer[0]) {
		case '+':
			command_plus();
			break;
		case 'd': printf("DEBUG  : %s",line_buffer+1); break;
		case 'i': printf("INFO   : %s",line_buffer+1); break;
		case 'w': printf("WARNING: %s",line_buffer+1); break;
		case 'e': printf("ERROR  : %s",line_buffer+1); break;
		case 'f': printf("process failed\n"); return;
		case 's': printf("process succeeded\n"); return;
		}
	}
}
