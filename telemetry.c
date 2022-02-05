#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <stdio.h>
#include <stdlib.h>
#include "telemetry.h"
#include "osdconfig.h"

void telemetry_init(telemetry_data_t *td) {
	td->validmsgsrx = 0;
	td->datarx = 0;

	td->voltage = 0;
	td->ampere = 0;
	td->mah = 0;
	td->msl_altitude = 0;
	td->rel_altitude = 0;
	td->longitude = 0;
	td->latitude = 0;
	td->heading = 0;
	td->cog = 0;
	td->speed = 0;
	td->airspeed = 0;
	td->throttle = 0;
	td->roll = 0;
	td->pitch = 0;
	td->sats = 0;
	td->fix = 0;
	td->hdop=0;
	td->armed = 255;
	td->rssi = 0;
	td->home_fix = 0;

	td->mav_climb = 0;
	td->vx=0;
	td->vy=0;
	td->vz=0;

#ifdef FRSKY
	td->x = 0;
	td->y = 0;
	td->z = 0;
	td->ew = 0;
	td->ns = 0;
#endif

#ifdef MAVLINK
    td->mav_flightmode = 255;
 //   td->mav_climb = 0;
	td->version=0;
	td->vendor=0;
	td->product=0;
//	td->vx=0;
//	td->vy=0;
//	td->vz=0;
//	td->hdop = 0;
	td->servo1 = 1500;
	td->mission_current_seq = 0;
	td->SP = 0;
	td->SE = 0;
	td->SH = 0;
	td->total_amps = 0;
#endif

#ifdef LTM
// ltm S frame
   	 td->ltm_status = 0;
   	 td->ltm_failsafe = 0;
   	 td->ltm_flightmode = 0;
// ltm N frame
   	 td->ltm_gpsmode = 0;
   	 td->ltm_navmode = 0;
   	 td->ltm_navaction = 0;
    	td->ltm_wpnumber = 0;
    	td->ltm_naverror = 0;
// ltm X frame
//   td->ltm_hdop = 0;
    	td->ltm_hw_status = 0;
    	td->ltm_x_counter = 0;
    	td->ltm_disarm_reason = 0;
// ltm O frame
   	 td->ltm_home_altitude = 0;
    	td->ltm_home_longitude = 0;
    	td->ltm_home_latitude = 0;
#endif
}
