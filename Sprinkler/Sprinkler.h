/* LoraSprinkler Firmware
 * Copyright (C) 2020 by Jeff McDonald (netadept@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/*
 * sprinkler.h 
 * Description: Sprinkler object header definition for use by the controller to send data to the remote nodes (zones)
 */

#ifndef Sprinkler_h
#define Sprinkler_h

#include "Arduino.h"

#define MS_IN_DAY   86400000
#define MS_IN_HOUR  3600000
#define MS_IN_MIN   60000

// each zone is its own class
class Sprinkler {
  private:

    // today: Internal tracker of current day should be updated daily
    uint8_t todayd, todayh, todaym, num_entries;

    // sname[64]: Sprinkler Name
    char sname[64];
	
    // debug: Used to pass values to the Serial.print for debugging
    int debug;
	
    /* schedule[100][2] consists of up to 100 sprinkler events per week
     *   consists of start time and duration offsets in milliseconds, computed from the schedule passed into constructor
     *   human readable code of "[Day of Week][hour][hour][minute][minute]" followed by "[zero-padded duration in minutes]" with day of week a 0-6 integer representing SUN-SAT respectively and hours in military time
     *   e.g. [["00400","00005"],["01600","00030"],["30400","00060"]]
	 */
    unsigned long int schedule [][2];

    /* function day_offset
     *   calculates day offset based on day of week provided, and wraps around the week.
     *   e.g. mon -> wed is 2 days, fri -> mon is 3 days
     */
    int day_offset(int zend);

    /* function parse_schedule_entry
         takes the human readable code "[day of week][hour][min]" and turns it into a millisecond delay from compile-time
    */
    unsigned long int parse_schedule_entry(char entry[6]);
    
  public:
    
    /* Constructor: requires Zone Name, initial schedule, compile-time hour, compile-time day, compile-time minute
	 *   NOTE: compile-time params to be replace by RTC when modules are added
	 */
	Sprinkler(char sname[64], char schedule [][2][6], int starth, int startd, int startm);
    
	/* function get_num_entries
	 *   getter for private num_entries attribute
	 */
	char * get_num_entries();
	
	/* function get_name
	 *   getter for private sname attribute
	 */
    char * get_name();
	
	/* function set_name
	 *   setter for private sname attribute
	 */
	void set_name(char * name);
    
	/* function get_next_start (RTC time in millis) 
	 *   returns next scheduled start time
	 *   NOTE: currently not implemented until RTC and until determined if node/zones will use this class
	 */
    unsigned long int get_next_start(unsigned long int currentMillis);
    
	/* function new_day
	 *   increments the private attribute todayd and factors in weekly modulus
	 * 	 NOTE: currently not used until determined if node/zones will use this class
	 */ 
    void new_day();
    
	/* function get_schedule
	 *   getter for schedule array. updates schedule array passed by reference
	 */ 
    void get_schedule(unsigned long int schedule[][2], int schedule_size);
    
	/* function get_debug
	 *   temporary function to pass values out of the object for use in Serial.print
	 */
    int get_debug();
};
#endif
