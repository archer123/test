#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define NO_IF_MATCH 0
#define IF_MATCH 1
#define FORKED 1
#define UNFORKED 0
#define TEST_SERVER_1 1
#define TEST_SERVER_2 2
#define TEST_SERVER_3 3
#define TEST_SERVER_4 4
#define TEST_SERVER_5 5
#define TEST_SERVER_6 6
#define NO_SIGNAL 0
#define SHUTDOWN 1
#define SYSTEM_IDLE 0
#define SYSTEM_BUSY 1
#define MAX_PORT 65535

/* GLOBAL VARIABLE: need to be used by CtrlHandler
 * which only accepts int, without returning anything */
int static input_signal = NO_SIGNAL;
int cpt=0;


/* FUNCTIONS */


/*char* tcpdump(char** arg_list){
	fprintf(stdout, "tcpdump_start_at=%lu\n", (unsigned long)time(NULL));

	int pid = getpid();
	char* command = malloc(sizeof(char)*256);
	sprintf(command, "sudo tcpdump -S 'tcp port %s' -w AP%d.pcap &",
		arg_list[4], pid);
	//printf("command %s \n", command);
	return(command);
}
*/

// char* analyse(char** arg_list){
// 	int pid = getpid();
// 	char* command = malloc(sizeof(char)*256);
// 	sprintf(command, "killall tcpdump; tshark -nr AP%d.pcap -Y 'not tcp.analysis.retransmission and not tcp.analysis.fast_retransmission and tcp.len > 0' -w _AP%d.pcapng &&\
// 	  editcap -F libpcap -T ether _AP%d.pcapng _AP%d.pcap && python analyse.py _AP%d.pcap &",
// 		pid, pid, pid, pid, pid);
// 	return(command);
// }


void launch_measurement(char** arg_list, char* port1, char* port2, char* port3, char* port4, char* port5, char* port6, int which_server, int *system_status){
	fprintf(stdout, "starting_timestamp_measurement=%lu\n", (unsigned long)time(NULL));

	char* command = malloc(sizeof(char)*256);
	char* command2 = malloc(sizeof(char)*256);
	char* command3 = malloc(sizeof(char)*256);
	if (!strcmp(arg_list[0], "iperf3")){
		strncpy(port1, arg_list[4], strlen(arg_list[4]));
		strncpy(port2, arg_list[4], strlen(arg_list[4]));
		//printf("%s %s %s %s %s %s.\n", arg_list[0], arg_list[1], arg_list[2], arg_list[3], arg_list[4], arg_list[5]);
		char* ptr;
		int tmp_port;

		tmp_port = strtol(arg_list[4], &ptr, 10);

		if (tmp_port + 500 < MAX_PORT){
			tmp_port = tmp_port + 100;
			sprintf(port2, "%d", tmp_port);
			tmp_port = tmp_port + 100;
			sprintf(port3, "%d", tmp_port);
			tmp_port = tmp_port + 100;
			sprintf(port4, "%d", tmp_port);
			tmp_port = tmp_port + 100;
			sprintf(port5, "%d", tmp_port);
			tmp_port = tmp_port + 100;
			sprintf(port6, "%d", tmp_port);
		} else {
			printf("[SYSTEM] error in the port range selection\n");
			exit(-10);
		}

		switch (which_server) {
			case TEST_SERVER_1:
				*system_status = SYSTEM_BUSY;
				arg_list[4] = port1;
				break;

			case TEST_SERVER_2:
				*system_status = SYSTEM_BUSY;
				arg_list[4] = port2;
				break;
			case TEST_SERVER_3:
				*system_status = SYSTEM_BUSY;
				arg_list[4] = port3;
				break;

			case TEST_SERVER_4:
				*system_status = SYSTEM_BUSY;
				arg_list[4] = port4;
				break;
			case TEST_SERVER_5:
				*system_status = SYSTEM_BUSY;
				arg_list[4] = port5;
				break;

			case TEST_SERVER_6:
				*system_status = SYSTEM_BUSY;
				arg_list[4] = port6;
				break;

			default:
				printf("[SYSTEM] error, unable to launch the test\n");
				exit(-20);
		}
		fprintf(stdout, "tcpdump_start_at=%lu\n", (unsigned long)time(NULL));

		int pid = getpid();
		sprintf(command2, "sudo tcpdump -S 'tcp port %s' -w AP%d.pcap &", arg_list[4], pid);
		system(command2);
		system("sleep 1");
		printf("delay 1 s to make sure tcpdump cap whole data\n");
		sprintf(command, "%s %s %s %s %s %d", arg_list[0], arg_list[1], arg_list[2], arg_list[3], arg_list[4], arg_list[5]);
		//printf("command %s \n", command);
		system(command);

	} else {

		//execvp (arg_list[0],  arg_list);
		int pid = getpid();
		sprintf(command2, "sudo tcpdump -S 'tcp port %s' -w AP%d.pcap &", arg_list[4], pid);
		system(command2);
		system("sleep 1");
		printf("delay 1 s to make sure tcpdump cap whole data\n");
		sprintf(command, "%s %s %s %s %s %d", arg_list[0], arg_list[1], arg_list[2], arg_list[3], arg_list[4], arg_list[5]);
		system(command);

	}
}


void launch_saturator(char* reliable_ip, char* reliabe_dev, char* test_ip, char* test_dev, char* server_ip){
	fprintf(stdout, "saturator_start_at=%lu\n", (unsigned long)time(NULL));
	char* saturator = "./saturatr";
	char* command = malloc(sizeof(char)*256);
	sprintf(command, "sudo %s %s %s %s %s %s", saturator, reliable_ip, reliabe_dev, test_ip, test_dev, server_ip);
	//printf("command %s \n", command);
	system(command);
}


void CtrlCHandler (int dummy) {

	input_signal = SHUTDOWN;
	printf("\n[SYSTEM] session terminated by the user\n\tTERMINATING THE SESSION...\n\tKILLING PROCESS: %d\n\n", getpid());
	system("killall tcpdump");
	fprintf(stdout, "\nclosing_timestamp=%lu\n\n", (unsigned long)time(NULL));
}

/* MAIN */
int main (int argc, char **argv){

	int devices, n_active_if, i;
	struct ifconf config;
	struct ifreq ifreq[128];
	int ret;

	pid_t childPID;

	char* sel_if_name = malloc(sizeof(char)*256);
	char* sel_if_addr = malloc(sizeof(char)*256);
	char* iperf_port1 = malloc(sizeof(char)*6);
	char* iperf_port2 = malloc(sizeof(char)*6);
	char* iperf_port3 = malloc(sizeof(char)*6);
	char* iperf_port4 = malloc(sizeof(char)*6);
	char* iperf_port5 = malloc(sizeof(char)*6);
	char* iperf_port6 = malloc(sizeof(char)*6);

	char* server_ip = malloc(sizeof(char)*256);
	server_ip = "132.227.122.38";
	//server_ip = "192.168.1.52";

	int static exec_status = UNFORKED;
	int static flag = NO_IF_MATCH;
	int test_server = TEST_SERVER_1;
	int sys_status = SYSTEM_IDLE;

	char* default_argv_list[] = {
		"iperf3",
		"-c",
		"132.227.122.38",
		//"192.168.1.52",
		"-p",
		"5000",
		NULL
	};

	char** user_argv_list;
	int user_argc = 0;

	fprintf(stdout, "starting_timestamp=%lu\n\n", (unsigned long)time(NULL));

	if (argc > 2){
		user_argc = argc - 2;
		user_argv_list = malloc(sizeof(char*)*(user_argc+1));
		for (int i_arg = 2; i_arg < argc; i_arg ++){
			user_argv_list[i_arg-2] = malloc(sizeof(char)*strlen(argv[i_arg]));
			user_argv_list[i_arg-2] = argv[i_arg];
		}
		user_argv_list[user_argc+1] = malloc(sizeof(NULL));
		user_argv_list[user_argc+1] = NULL;
	}

	printf("car-client: $ ");
	for (int j = 0; j < user_argc; j ++) {
		printf("%s ", user_argv_list[j]);
	}
	printf("\n\n");

	/* Initialize the structure to 0s */
	memset(&config, 0, sizeof(struct ifconf));

	/* Open a TCP socket (SOCK_STREAM) to issue the ioctl */
	devices = socket(AF_INET, SOCK_STREAM, 0);
	/* Handling errors with the socket opening */
	if (devices < 0) {
		perror("cannot open socket");
		return -1;
	}

	printf("TOOL STARTED\n\n");
	printf("[SYSTEM] father process PID: %d\n", getpid());
	signal(SIGINT, CtrlCHandler);

	while(1){

		/* if ctrl+c detected, kill everything, no matter what */
		if (input_signal == SHUTDOWN && exec_status == FORKED && childPID > 0) {
			kill( childPID, SIGKILL );
			exec_status = UNFORKED;
			sys_status = SYSTEM_IDLE;

		}
		if (input_signal == SHUTDOWN && exec_status == UNFORKED) {
			char* command = malloc(sizeof(char)*256);
			sprintf(command, "python analysis.py %s", server_ip);
			system(command);
			printf("[SYSTEM] child process killed, closing father process\n");
			return 5;
		}
		/*******************************************************/

		/* reset the interface match flag at every iteration */
		flag = NO_IF_MATCH;

		/* fill-in the config request structure, including a buffer to hold the answer */
		config.ifc_buf = (char *) ifreq;
		config.ifc_len = 128 * sizeof(struct ifreq);

		/* issue the system call */
		ret = ioctl(devices, SIOCGIFCONF, (char *) &config);
		if (ret < 0) {
			perror("ioctl failed\n");
			close(devices);
			return -2;
		}

		n_active_if = config.ifc_len / (sizeof(struct ifreq));

		/* loop over the active interfaces */
		for (i = 0; i < n_active_if; i++){
			sel_if_name = ifreq[i].ifr_name;
			sel_if_addr = inet_ntoa(((struct sockaddr_in *)&ifreq[i].ifr_addr)->sin_addr);

			if(argc > 1){
				if (strcmp(argv[1], ifreq[i].ifr_name) == 0){

					flag = IF_MATCH;

					/* fork the process and launch the measurements from the child process */
					if (exec_status == UNFORKED) {
						exec_status = FORKED;
						printf("\n[SYSTEM] forking a child process to run the measurements\n");

						/* need to alternate between 6 different iperf servers */
						if (test_server == TEST_SERVER_1) {

							test_server = TEST_SERVER_2;

						} else if (test_server == TEST_SERVER_2) {

							test_server = TEST_SERVER_3;

						} else if (test_server == TEST_SERVER_3) {

							test_server = TEST_SERVER_4;

						} else if (test_server == TEST_SERVER_4) {

							test_server = TEST_SERVER_5;

						} else if (test_server == TEST_SERVER_5) {

							test_server = TEST_SERVER_6;

						} else if (test_server == TEST_SERVER_6) {

							test_server = TEST_SERVER_1;

						}

						/* fork the program and execute an external function */
						childPID = fork();
						/* handle the errors */
						if (childPID < -1){
							perror("ERROR: ");
							exit(-1);
						}
						if (childPID == 0) {
							/* I am in the child process, launch the measurements */
							printf("[SYSTEM] child process PID: %d\n", getpid());

							if (sys_status == SYSTEM_IDLE){

								printf("\ncar-client: launcing the measurement\n\n");
								//printf("\nlauncing saturater\n\n");
								//launch_saturator(sel_if_addr, sel_if_name, sel_if_addr, sel_if_name, server_ip);
								if (user_argc > 0) {
									launch_measurement(user_argv_list, iperf_port1, iperf_port2, iperf_port3, iperf_port4, iperf_port5, iperf_port6, test_server, &sys_status);
								} else {
									printf("car-client: default option chosen\n");
									launch_measurement(default_argv_list, iperf_port1, iperf_port2, iperf_port3, iperf_port4, iperf_port5, iperf_port6, test_server, &sys_status);
								}
							} else {
								printf("\ncar-client: ERROR, the system is busy\n");
								kill( getpid(), SIGKILL );
							}
						}
					}
				}
			} else {
				/* get and print ip address and name of all the active ifs */
				ifreq[i].ifr_addr.sa_family = AF_INET;
				ioctl(devices, SIOCGIFADDR, &ifreq[i]);
				sel_if_addr = inet_ntoa(((struct sockaddr_in *)&ifreq[i].ifr_addr)->sin_addr);
				printf("\nInterface: %s\n", sel_if_name);
				printf("Address: %s\n", sel_if_addr);
			}
		}
		/* if the interface is not active anymore, kill the measurement in the child process */
		if (argc > 1 && flag == NO_IF_MATCH) {
			/* the father process kills the child process */
			if (exec_status == FORKED && childPID > 0) {

				kill( childPID, SIGKILL );
				system("sudo killall iperf3");
				system("sudo killall tcpdump");

				exec_status = UNFORKED;
				sys_status = SYSTEM_IDLE;
				printf("[SYSTEM] process %d unforked\n", childPID);
				fprintf(stdout, "closing_timestamp_measurement=%lu\n\n", (unsigned long)time(NULL));
			}
		}
		if (argc == 1)
			return 0;
	}


	close(devices);



	/* freeing allocated memory*/
	for (int free_i = 0; free_i < user_argc; free_i ++) {
		free(user_argv_list[free_i]);
	}
	free(user_argv_list);
	free(iperf_port1);
	free(iperf_port2);

	free(sel_if_name);
	free(sel_if_addr);
	return 0;
}
