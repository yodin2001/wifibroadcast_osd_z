/*
Copyright (c) 2015, befinitiv
Copyright (c) 2012, Broadcom Europe Ltd
modified by Samuel Brucksch https://github.com/SamuelBrucksch/wifibroadcast_osd
modified by Rodizio

All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the copyright holder nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <locale.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>


#include "render.h"
#include "osdconfig.h"
#include "telemetry.h"
#ifdef FRSKY
#include "frsky.h"
#elif defined(LTM)
#include "ltm.h"
#elif defined(MAVLINK)
#include "mavlink.h"
#elif defined(SMARTPORT)
#include "smartport.h"
#elif defined(VOT)
#include "vot.h"
#endif

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    return milliseconds;
}

fd_set set;

struct timeval timeout;


uint8_t telem_buf[263];
struct pollfd fds1[1];
int o_res;

int open_udp_socket_for_rx(int port, struct pollfd *pollfd_struct)
{
    struct sockaddr_in saddr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0){
        perror("Error opening socket");
        exit(1);
    }

    printf("OSD_PORT=%d\n", port);

    int optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    bzero((char *) &saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons((unsigned short)port);

    if (bind(fd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0)
    {
        perror("Bind error");
        exit(1);
    }

    pollfd_struct[0].fd = fd;
    pollfd_struct[0].events = POLLIN;

    return fd;
}


void get_rssi(telemetry_data_t *td)
{
    int rc = poll(fds1, 1, 10);

    if (fds1[0].revents & (POLLERR | POLLNVAL))
    {
        fprintf(stderr, "socket error!");
        exit(1);
    }

        if (fds1[0].revents & POLLIN){
            ssize_t rsize;
            while((rsize = recv(o_res, telem_buf, sizeof(telem_buf), 0)) >= 0)
            {
                //fprintf(stderr, "recieved %d bytes\n", rsize);
                //memcpy(&(td->rx_status) , &telem_buf, rsize);
                //fprintf(stderr,"recieved blocks %d\n", td->rx_status->received_block_cnt ); 
                td->rx_status = (wifibroadcast_rx_status_forward_t *)&telem_buf;
                //wifibroadcast_rx_status_forward_t *test = (wifibroadcast_rx_status_forward_t *)&telem_buf;
                //fprintf(stderr,"value %d\n", test->cpuload_air); 
                
            }
            if (rsize < 0 && errno != EWOULDBLOCK){
                perror("Error receiving packet");
                exit(1);
            }
        }

    
}



int main(int argc, char *argv[]) {
    fprintf(stderr,"OSD started\n=====================================\n\n");

    setpriority(PRIO_PROCESS, 0, 10);

    setlocale(LC_ALL, "en_GB.UTF-8");

    uint8_t buf[263]; // Mavlink maximum packet length
    size_t n;

    long long fpscount_ts = 0;
    long long fpscount_ts_last = 0;
    int fpscount = 0;
    int fpscount_last = 0;
    int fps;

    int do_render = 0;
    int counter = 0;

    
    o_res = open_udp_socket_for_rx(5003, (struct pollfd *) &fds1);
    if(fcntl(o_res, F_SETFL, fcntl(o_res, F_GETFL, 0) | O_NONBLOCK) < 0)
    {
        perror("Unable to set socket into nonblocked mode");
        exit(1);
    }



    


#ifdef FRSKY
    frsky_state_t fs;
#endif

    struct stat fdstatus;
    signal(SIGPIPE, SIG_IGN);
    char fifonam[100];
    sprintf(fifonam, "/root/telemetryfifo1");

    int readfd;
    readfd = open(fifonam, O_RDONLY | O_NONBLOCK);
    if(-1==readfd) {
        perror("ERROR: Could not open /root/telemetryfifo1");
        exit(EXIT_FAILURE);
    }
    if(-1==fstat(readfd, &fdstatus)) {
        perror("ERROR: fstat /root/telemetryfifo1");
        close(readfd);
        exit(EXIT_FAILURE);
    }

    //______UDP TELEMETRY_____________
    int fd;
    struct pollfd fds[1];
    char *osd_port = getenv("OSD_PORT");
    memset(fds, '\0', sizeof(fds));
    fd = open_udp_socket_for_rx(osd_port == NULL ? 14550 : atoi(osd_port), (struct pollfd *) &fds);
    if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0)
    {
        perror("Unable to set socket into nonblocked mode");
        exit(1);
    }

    
    //----------------------------------


    fprintf(stderr,"OSD: Initializing sharedmem ...\n");
    telemetry_data_t td;
    telemetry_init(&td);
    fprintf(stderr,"OSD: Sharedmem init done\n");


    fprintf(stderr,"OSD: Initializing render engine ...\n");
    render_init();
    fprintf(stderr,"OSD: Render init done\n");

    long long prev_time = current_timestamp();
    long long prev_time2 = current_timestamp();


    while(1) {

    get_rssi(&td);

//______UDP TELEMETRY_____________
        int rc = poll(fds, 1, 50);

        if (rc < 0){
            if (errno == EINTR || errno == EAGAIN) continue;
            perror("Poll error");
            exit(1);
        }

        if (fds[0].revents & (POLLERR | POLLNVAL))
        {
            fprintf(stderr, "socket error!");
            exit(1);
        }

        if (fds[0].revents & POLLIN){
            ssize_t rsize;
            while((rsize = recv(fd, buf, sizeof(buf), 0)) >= 0)
            {
                //fprintf(stderr, "recieved %d bytes\n", rsize);
#ifdef FRSKY
	        	frsky_parse_buffer(&fs, &td, buf, rsize);
#elif defined(LTM)
	        	do_render = ltm_read(&td, buf, rsize);
#elif defined(MAVLINK)
	        	do_render = mavlink_read(&td, buf, rsize);
#elif defined(SMARTPORT)
	        	smartport_read(&td, buf, rsize);
#elif defined(VOT)
	        	do_render =  vot_read(&td, buf, rsize);
                fprintf(stderr, "\n");
#endif
            }
            if (rsize < 0 && errno != EWOULDBLOCK){
                perror("Error receiving packet");
                exit(1);
            }
        }
//_________________________________

	    //}
	    counter++;
//	    fprintf(stderr,"OSD: counter: %d\n",counter);
	    // render only if we have data that needs to be processed as quick as possible (attitude)
	    // or if three iterations (~150ms) passed without rendering
	    if ((do_render == 1) || (counter == 3)) {
//		fprintf(stderr," rendering! ");
		prev_time = current_timestamp();
		fpscount++;
		render(&td, fps);
		long long took = current_timestamp() - prev_time;
//		fprintf(stderr,"Render took %lldms\n", took);
		do_render = 0;
		counter = 0;
	    }
        
//		long long took = current_timestamp() - prev_time;
//		fprintf(stderr,"while took %lldms\n", took);

		long long fpscount_timer = current_timestamp() - fpscount_ts_last;
		if (fpscount_timer > 2000) {
		    fpscount_ts_last = current_timestamp();
		    fps = (fpscount - fpscount_last) / 2;
		    fpscount_last = fpscount;
		    //fprintf(stderr,"OSD FPS: %d\n", fps);
		}
    }
    return 0;
}
