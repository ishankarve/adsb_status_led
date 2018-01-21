/*
 * Coded by Ishan Anant Karve, Aug 2014
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <string.h>
#include <syslog.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>

//--------------------------------------------------------------------------------
#include <wiringPi.h> // Include WiringPi library!
//-------------------------------------------------------------------------
#define BUFFER_SIZE (256 * 1024)  /* 256 KB */
#define URL_SIZE    256
#define MAXLEN 		80
//--------------------------------------------------------------------------------
#define MDA_STATUS_LED	2
//--------------------------------------------------------------------------------
void gpio_init(void)
{
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers
    pinMode(MDA_STATUS_LED, OUTPUT); // Set LED as PWM output
}
void toggle_led(int times,int interval)
{
    int ik=0;
    while (ik<=times)
    {
        digitalWrite(MDA_STATUS_LED, HIGH); // Turn LED ON
        delay(interval); // Wait 200ms
        digitalWrite(MDA_STATUS_LED, LOW); // Turn LED OFF
        delay(interval); // Wait 200ms again
        ik++;
    }
}
//--------------------------------------------------------------------------------
int readline(int fd, char ** out)
{
    /* Keep reading till header "MSG" is received
     * store it in buffer
     * keep reading till newline is encountered
     * exit function
     */
    int buf_size = 1024;
    int bytesloaded = 0;
    int ret;
    char buf;
    char * buffer = malloc(buf_size);
    if (NULL == buffer)
		{
			free(buffer);
			return -1;
		}
    //loop till header is received
    while (1)
    {
        bytesloaded=0;
        bzero(buffer,1024);
        ret = read(fd, &buf, 1);
        if (ret < 1)
        {
            free(buffer);
            return -1;
        }
        if (buf=='M')  	//Got M; now wait for S
        {
            bytesloaded=0;
            buffer[bytesloaded] = buf;
            bytesloaded++;
            ret = read(fd, &buf, 1);
            if (ret < 1)
            {
                free(buffer);
                return -1;
            }
            if (buf=='S')  	//Got S; now wait for G
            {
                buffer[bytesloaded] = buf;
                bytesloaded++;
                ret = read(fd, &buf, 1);
                if (ret < 1)
                {
                    free(buffer);
                    return -1;
                }
                if (buf=='G')  	//Got G; now wait for CR-LF
                {
                    buffer[bytesloaded] = buf;
                    bytesloaded++;
                    while(1)
                    {
                        ret = read(fd, &buf, 1);
                        if (ret < 1)
                        {
                            free(buffer);
                            return -1;
                        }
                        if (buf=='\r')
                            break;
                        buffer[bytesloaded] = buf;
                        bytesloaded++;
                    }
                    buffer[bytesloaded] = '\0';
                    *out = buffer; // complete line
                    
                    return bytesloaded;
                }
            }
        }
    }
}
int main(int argc, char *argv[])
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char* tbuf;
    int ret;
    int i,j;
    pid_t process_id = 0;
    pid_t sid = 0;
    // Create child process
    process_id = fork();
    // Indication of fork() failure
    if (process_id < 0)
    {
        printf("Error creating a daemon!\n");
        // Return failure in exit status
        exit(1);
    }
    // PARENT PROCESS. Need to kill it.
    if (process_id > 0)
    {
        printf("process_id of child process %d \n", process_id);
        // return success in exit status
        exit(0);
    }
    //unmask the file mode
    umask(0);
//set new session
    sid = setsid();
    if(sid < 0)
    {
// Return failure
        exit(1);
    }
// Change the current working directory to root.
    chdir("/");
    // initialise GPIO
    gpio_init();
    syslog(LOG_INFO,"%s","Execution Started.\n");
    fprintf(stderr, "\nCoded and conceptualised by Cdr Ishan Anant Karve <ishan.karve@gmail.com>.\nProgram check connectivity with local ADSB server.\n\n");
    // Close stdin. stdout and stderr
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
		
		
    }
    bzero(&(serv_addr.sin_zero),8);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(30003);
    //serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* Now connect to the server */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) == -1)
    {
        perror("ERROR connecting");
        exit(1);
    }
    /* Now read server response */
    while (1)
    {
        ret = readline(sockfd, &tbuf);
        if (ret < 0)
        {
            printf("\r\nERROR reading from socket!");
           // exit(1);
			//goto TryAgain;
        }
		else
		{
			//printf ("%d\n",ret);
			toggle_led(2,25);
		}
		
    }
    return 0;
}
