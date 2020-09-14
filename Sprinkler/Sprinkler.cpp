#include "Arduino.h"
#include "Sprinkler.h"

// each zone is its own class
Sprinkler::Sprinkler(char sname[64], char schedule [][2][6], int startd, int starth, int startm) {
  strcpy(this->sname, sname);
    this->todayd = startd;
    this->todayh = starth;
    this->todaym = startm;
  int num_zones = (sizeof (schedule)/sizeof (schedule[0]));
//  debug = num_zones;
  for (int i = 0; i < num_zones; i++) {
    this->schedule[i][0] = parse_schedule_entry(schedule[i][0]);
    this->schedule[i][1] = this->schedule[i][0] + (atoi(schedule[i][1]) * MS_IN_MIN);
    
  }
}

int Sprinkler::day_offset(int zend) {
    if (todayd > zend) {
        return (zend + 7 - todayd);
    } else {
        return (zend - todayd);
    }
}

    /* function parse_schedule_entry
          takes the human readable code "[day of week][hour][min]" and turns it into a millisecond delay from compile-time

    */
unsigned long int Sprinkler::parse_schedule_entry(char entry[6]) {
      // parse it up
    int d = 0;
    int e = 0;
    char day_of_week;
    char entry_hour[3];
    char entry_minute[3];
    int parse_hour, parse_minute = 0;
    // get day of week
    day_of_week = entry[0];
    strncpy(entry_hour,entry+1,2);
    strncpy(entry_minute,entry+3,2);

    //debug = day_offset(day_of_week - '0');
    debug = atoi(entry_hour);
    return ((day_offset(day_of_week - '0') * MS_IN_DAY) + (atoi(entry_hour) * MS_IN_HOUR) + (atoi(entry_minute) * MS_IN_MIN) - (todayh * MS_IN_HOUR) - (todaym * MS_IN_MIN));
}

char * Sprinkler::get_name() {
      return this->sname;
}
void Sprinkler::set_name(char * name){
	strcpy(sname, name);
}

unsigned long int Sprinkler::get_next_start(unsigned long int currentMillis) {
      return currentMillis;
}

void Sprinkler::new_day() {
    this->todayd = (this->todayd + 1 == 7 ? 0 : this->todayd + 1);
}

void Sprinkler::get_schedule(unsigned long int schedule[][2], int schedule_size) {
      //int schedule_size = sizeof(this->schedule) / sizeof (this->schedule[0]);
      for(int i = 0; i < schedule_size; i++){
        schedule[i][0] = this->schedule[i][0];
        schedule[i][1] = this->schedule[i][1];
      }
}
int Sprinkler::get_debug(){
      return debug;
}

