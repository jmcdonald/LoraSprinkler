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

/* Sprinkler.cpp
 * Description: Sprinkler Object definition for use with the controller to send data to the remote nodes (zones)
 */
 
#include "Arduino.h"
#include "Sprinkler.h"

Sprinkler::Sprinkler(char sname[64], char schedule [][2][6], int startd, int starth, int startm) {
  strcpy(this->sname, sname);
  this->todayd = startd;
  this->todayh = starth;
  this->todaym = startm;
  num_entries = (sizeof (schedule)/sizeof (schedule[0]));
  for (int i = 0; i < num_entries; i++) {
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

unsigned long int Sprinkler::parse_schedule_entry(char entry[6]) {

    int d = 0;
    int e = 0;
    char day_of_week;
    char entry_hour[3];
    char entry_minute[3];
    int parse_hour, parse_minute = 0;
    day_of_week = entry[0];
    strncpy(entry_hour,entry+1,2);
    strncpy(entry_minute,entry+3,2);
    return ((day_offset(day_of_week - '0') * MS_IN_DAY) + (atoi(entry_hour) * MS_IN_HOUR) + (atoi(entry_minute) * MS_IN_MIN) - (todayh * MS_IN_HOUR) - (todaym * MS_IN_MIN));
}

char * Sprinkler::get_num_entries(){
	return (char *)num_entries;
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
      for(int i = 0; i < schedule_size; i++){
        schedule[i][0] = this->schedule[i][0];
        schedule[i][1] = this->schedule[i][1];
      }
}

int Sprinkler::get_debug(){
      return debug;
}

