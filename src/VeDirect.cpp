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

static const char *EMPTY_STRING = "";

int _read_vedirect(char *output, const char *tag, const char *line)
{
    char str[80];
    strcpy(str, line);
    char *token;
    token = strtok(str, "\t");
    if (token && strcmp(tag, token) == 0)
    {
        token = strtok(NULL, "\t");
        if (token)
        {
            strcpy(output, token);
            return -1;
        }
    }
    return 0;
}

int read_vedirect_int(int &v, const char *tag, const char *line)
{
    char token[80];
    if (_read_vedirect(token, tag, line))
    {
        if (strcmp("---", token) == 0)
        {
            return 0; // undefined value in ve.direct dialect
        }
        else
        {
            v = strtol(token, NULL, 0);
            return -1;
        }
    }
    return 0;
}

int read_vedirect(double &v, double precision, const char *tag, const char *line)
{
    int iv = 0;
    if (read_vedirect_int(iv, tag, line))
    {
        v = iv * precision;
        return -1;
    }
    return 0;
}

int read_vedirect_onoff(bool &v, const char *tag, const char *line)
{
    char token[80];
    if (_read_vedirect(token, tag, line))
    {
        v = strcmp("ON", token);
        return -1;
    }
    return 0;
}

VEDirectObject::VEDirectObject(const VEDirectValueDefinition *definition, int len) : valid(0), n_fields(len), fields(definition)
{
    i_values = new int[n_fields];
    last_time = new unsigned long[n_fields];
    s_values = new char *[n_fields];
    for (int i = 0; i < n_fields; i++)
        s_values[i] = NULL;
    reset();
}

VEDirectObject::~VEDirectObject()
{
    delete i_values;
    delete last_time;
    for (int i = 0; i < n_fields; i++)
    {
        if (s_values[i])
            delete s_values[i];
    }
    delete s_values;
}

void VEDirectObject::reset()
{
    for (int i = 0; i < n_fields; i++)
    {
        last_time[i] = 0;
        i_values[i] = 0;
        if (s_values[i])
            delete s_values[i];
        s_values[i] = NULL;
    }
    valid = 0;
}

void VEDirectObject::print()
{
    Log::trace("New ve.direct object\n");
    for (int i = 0; i < n_fields; i++)
    {
        if (last_time[i])
            switch (fields[i].veType)
            {
            case VE_BOOLEAN:
                Log::trace("Field %d %s {%s}\n", i, fields[i].veName, i_values[i] ? "ON" : "OFF");
                break;
            case VE_NUMBER:
                if (fields[i].veUnit)
                    Log::trace("Field %d %s {%d %s}\n", i, fields[i].veName, i_values[i], fields[i].veUnit);
                else
                    Log::trace("Field %d %s {%d}\n", i, fields[i].veName, i_values[i]);
                break;
            case VE_STRING:
                Log::trace("Field %d %s {%s}\n", i, fields[i].veName, s_values[i]);
                break;
            default:
                break;
            }
    }
    Log::trace("End ve.direct object\n");
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
            if (read_vedirect_int(i_values[i], def.veName, line))
            {
                last_time[i] = time;
                valid++;
            }
        }
        break;
        case VEFieldType::VE_BOOLEAN:
        {
            bool res;
            if (read_vedirect_onoff(res, def.veName, line))
            {
                i_values[i] = res ? 1 : 0;
                last_time[i] = time;
                valid++;
            }
        }
        break;
        case VEFieldType::VE_STRING:
        {
            static char str[80];
            if (_read_vedirect(str, def.veName, line))
            {
                if (s_values[def.veIndex] == NULL)
                    delete s_values[def.veIndex];
                s_values[def.veIndex] = strdup(str);
                last_time[i] = time;
                valid++;
            }
        }
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
    if (s_values[index])
    {
        strcpy(value, s_values[index]);
        return -1;
    }
    return 0;
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