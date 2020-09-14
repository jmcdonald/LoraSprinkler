/*
 * sprinkler.h - a sprinkler object for use by the house controller LORA system
 * Created by Jeff McDonald
 * Revision #1
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
    int todayd, todayh, todaym;

    // sname[64]: Sprinkler Name
    char sname[64];
    //unsigned long int debug = 999;
    int debug;
    // schedule[100][2] consists of up to 100 sprinkler events per week
    //    consists of start time and duration offsets in milliseconds, computed from the schedule passed into constructor
    //    human readable code of "[Day of Week][hour][hour][minute][minute]" followed by "[zero-padded duration in minutes]" with day of week a 0-6 integer representing SUN-SAT respectively and hours in military time
    //    e.g. [["00400","00005"],["01600","00030"],["30400","00060"]]
    unsigned long int schedule [][2];

    /* function day_offset
          calculates day offset based on day of week provided, and wraps around the week.
          e.g. mon -> wed is 2 days, fri -> mon is 3 days
    */
    int day_offset(int zend);

    /* function parse_schedule_entry
          takes the human readable code "[day of week][hour][min]" and turns it into a millisecond delay from compile-time

    */
    unsigned long int parse_schedule_entry(char entry[6]);
    
  public:
    //unsigned long int debug;
    
    Sprinkler(char sname[64], char schedule [][2][6], int starth, int startd, int startm);
    	
    char * get_name();
	
	void set_name(char * name);
    
    unsigned long int get_next_start(unsigned long int currentMillis);
    
    void new_day();
    
    void get_schedule(unsigned long int schedule[][2], int schedule_size);
    
    int get_debug();
};
#endif
