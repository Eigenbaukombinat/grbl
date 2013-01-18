/*
  spindle_control.c - spindle control methods
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2012 Sungeun K. Jeon

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "settings.h"
#include "spindle_control.h"
#include "planner.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static uint8_t current_direction;

#define MAX 65535

void spindle_init()
{
  current_direction = 0;
  SPINDLE_ENABLE_DDR |= (1<<SPINDLE_ENABLE_BIT);
  SPINDLE_DIRECTION_DDR |= (1<<SPINDLE_DIRECTION_BIT);
  spindle_stop();
}

int spindle_on = 0, spindle_where = 0, spindle_up, spindle_mult = 2, spindle_initial;
void spindle_stop()
{
  spindle_up = -1;
}


ISR(TIMER1_OVF_vect)
{


   if (!spindle_on) {
    SPINDLE_ENABLE_PORT |= 1<<SPINDLE_ENABLE_BIT;
    spindle_on = 1;

    if (spindle_up > 0) {
      if (spindle_where < 32000) {
        if (spindle_initial > 128) {
          spindle_where+=spindle_mult;
          spindle_mult++;
        } else {
          spindle_initial+=8;
          spindle_where+=1;
        }
      }
    } else if (spindle_up < 0) {
      spindle_where-=32;
    }

    TCNT1  = (MAX) - spindle_where; // Reload timer with precalculated value
   } else {
     SPINDLE_ENABLE_PORT &= ~(1<<SPINDLE_ENABLE_BIT);
     spindle_on = 0;
     TCNT1  = MAX-2960;
   }

   if (spindle_up == -1 && spindle_where < 32) {
    TCCR1B = 0;
   }

}

void spindle_run(int8_t direction) //, uint16_t rpm)
{
  spindle_up = 1;
  spindle_initial = 0;
  spindle_where=16650;
  spindle_mult=1;
  TIMSK1 |= (1 << TOIE1); // Enable overflow interrupt
  TCNT1 = 0; // Preload timer with precalculated value
  TCCR1B |= (1 << CS10);
}
