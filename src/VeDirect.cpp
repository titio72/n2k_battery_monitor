/*
(C) 2022, Andrea Boni
This file is part of n2k_battery_monitor.
n2k_battery_monitor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
NMEARouter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with n2k_battery_monitor.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "VeDirect.h"
#include "Utils.h"
#include "Log.h"
#include <stdio.h>
#include <string.h>

/*
VE.Direct format:

PID     0xA381
V       12488
VS      12909
I       0
P       0
CE      -220375
SOC     196
TTG     -1
Alarm   ON
Relay   OFF
AR      4
BMV     712 Smart
FW      0408
MON     0
 */

int read_vedirect(double& v, double precision, const char* tag, const char* line, unsigned long& last_time) {
  char str[80];
  strcpy(str, line);
  char *token;
  token = strtok(str, "\t");
  if (token && strcmp(tag, token)==0) {
    token = strtok(NULL, "\t");
    if (token) {
      if (strcmp("---", token)==0) {
        return 0;
      } else {
        v = atof(token) * precision;
        last_time = _millis();
        return -1;
      }
    }
  }
  return 0;
}

int read_vedirect_int(int& v, const char* tag, const char* line, unsigned long& last_time) {
  char str[80];
  strcpy(str, line);
  char *token;
  token = strtok(str, "\t");
  if (token && strcmp(tag, token)==0) {
    //printf("'%s' '%s'\n", line, tag);
    token = strtok(NULL, "\t");
    if (token) {
      if (strcmp("---", token)==0) {
        return 0;
      } else {
        v = atoi(token);
        //printf("OK '%s' (%d)\n", tag, v);
        last_time = _millis();
        return -1;
      }
    }
  }
  return 0;
}

int read_vedirect_onoff(bool& v, const char* tag, const char* line, unsigned long& last_time) {
  char str[80];
  strcpy(str, line);
  char *token;
  token = strtok(str, "\t");
  if (token && strcmp(tag, token)==0) {
    token = strtok(NULL, "\t");
    if (token) {
      v = strcmp("ON", token);
      last_time = _millis();
      return -1;
    }
  }
  return 0;
}

VEDirectObject::VEDirectObject(const VEDirectValueDefinition *definition, int len) : valid(0), n_fields(len), fields(definition)
{
    i_values = new int[n_fields];
    last_time = new unsigned long[n_fields];
    // printf("New ve.direct object");
    Log::trace("New ve.direct object\n");
    for (int i = 0; i < len; i++)
    {
        // printf("Field %d %s %s\n", fields[i].veIndex, fields[i].veName, fields[i].veUnit);
        Log::trace("Field %d %s %s\n", fields[i].veIndex, fields[i].veName, fields[i].veUnit);
    }
    Log::trace("End ve.direct object\n");
    reset();
}

VEDirectObject::~VEDirectObject()
{
    delete i_values;
    delete last_time;
}

void VEDirectObject::load_VEDirect_key_value(const char *line, unsigned long time)
{
    for (int i = 0; i < BMV_N_FIELDS; i++)
    {
        const VEDirectValueDefinition def = BMV_FIELDS[i];
        switch (def.veType)
        {
        case VEFieldType::VE_NUMBER:
        {
            if (read_vedirect_int(i_values[i], def.veName, line, last_time[i]))
            {
                valid++;
                // printf("Read %s %d (%d)\n", def.veName, i_values[i], valid);
            }
        }
        break;
        case VEFieldType::VE_BOOLEAN:
        {
            bool res;
            if (read_vedirect_onoff(res, def.veName, line, last_time[i]))
            {
                i_values[i] = res ? 1 : 0;
                valid++;
            }
        }
        break;
        default:
            break;
        }
    }
}

int VEDirectObject::get_number_value(int &value, unsigned int index)
{
    if (index > n_fields)
        return 0;
    VEDirectValueDefinition field = fields[index];
    if (field.veIndex < BMV_N_FIELDS && last_time[field.veIndex])
    {
        value = i_values[field.veIndex];
        return -1;
    }
    else
    {
        return 0;
    }
}

int VEDirectObject::get_number_value(double &value, double precision, unsigned int index)
{
    if (index > n_fields)
        return 0;
    VEDirectValueDefinition field = fields[index];
    if (field.veIndex < BMV_N_FIELDS && last_time[field.veIndex])
    {
        value = i_values[field.veIndex] * precision;
        return -1;
    }
    else
    {
        return 0;
    }
}

int VEDirectObject::get_boolean_value(bool &value, unsigned int index)
{
    if (index > n_fields)
        return 0;
    VEDirectValueDefinition field = fields[index];
    if (field.veIndex < BMV_N_FIELDS && last_time[field.veIndex])
    {
        value = i_values[field.veIndex];
        return -1;
    }
    else
    {
        return 0;
    }
}

unsigned long VEDirectObject::get_last_timestamp(unsigned int index)
{
    if (index > n_fields)
        return 0;
    return last_time[index];
}

int VEDirectObject::get_string_value(char *value, unsigned int index)
{
    return 0; // unsupported for now
}

void VEDirectObject::reset()
{
    for (int i = 0; i < n_fields; i++)
        last_time[i] = 0;
    for (int i = 0; i < n_fields; i++)
        i_values[i] = 0;
    valid = 0;
}

bool VEDirectObject::is_valid()
{
    return valid;
}

/*
PID	0xA381
V	13406
VS	13152
I	0
P	0
CE	-89423
SOC	689
TTG	-1
Alarm	OFF
Relay	OFF
AR	0
BMV	712 Smart
FW	0413
MON	0
Checksum	y
H1	-277191
H2	-89430
H3	-137695
H4	21
H5	1
H6	-5966596
H7	30
H8	16200
H9	86935
H10	17
H11	71
H12	0
H15	22
H16	15394
H17	7749
H18	9056
Checksum
*/