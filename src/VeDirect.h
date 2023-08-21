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

#ifndef _VEDIRECT
#define _VEDIRECT

#include <math.h>

enum VEFieldType
{
    VE_STRING,
    VE_NUMBER,
    VE_BOOLEAN
};

class VEDirectValueDefinition
{
public:
    VEDirectValueDefinition(VEFieldType type, const char *veDirectName, unsigned int index) : veType(type), veName(veDirectName), veIndex(index){};
    VEDirectValueDefinition(VEFieldType type, const char *veDirectName, unsigned int index, const char *unit) : veType(type), veName(veDirectName), veIndex(index), veUnit(unit){};

    VEFieldType veType;
    const char *veName;
    unsigned int veIndex;
    const char *veUnit = NULL;
};

static const unsigned int BMV_N_FIELDS = 14;
static const VEDirectValueDefinition BMV_PID(VE_NUMBER, "PID", 0);
static const VEDirectValueDefinition BMV_VOLTAGE(VE_NUMBER, "V", 1, "mV");
static const VEDirectValueDefinition BMV_VOLTAGE_1(VE_NUMBER, "VS", 2, "mV");
static const VEDirectValueDefinition BMV_CURRENT(VE_NUMBER, "I", 3, "mA");
static const VEDirectValueDefinition BMV_CONSUMPTION(VE_NUMBER, "CE", 4, "mAh");
static const VEDirectValueDefinition BMV_SOC(VE_NUMBER, "SOC", 5, "1/1000");
static const VEDirectValueDefinition BMV_TIME_TO_GO(VE_NUMBER, "TTG", 6, "Minutes");
static const VEDirectValueDefinition BMV_ALARM(VE_BOOLEAN, "Alarm", 7);
static const VEDirectValueDefinition BMV_RELAY(VE_BOOLEAN, "Relay", 8);
static const VEDirectValueDefinition BMV_ALARM_REASON(VE_NUMBER, "AR", 9, "Enum");
static const VEDirectValueDefinition BMV_FIRMWARE(VE_STRING, "FW", 10);
static const VEDirectValueDefinition BMV_MONITOR_MODE(VE_NUMBER, "MON", 11, "Enum");
static const VEDirectValueDefinition BMV_TEMPERATURE(VE_NUMBER, "T", 12, "C");
static const VEDirectValueDefinition BMV_BMV(VE_STRING, "BMV", 13);
static const VEDirectValueDefinition BMV_FIELDS[BMV_N_FIELDS] = {
    BMV_PID,
    BMV_VOLTAGE,
    BMV_VOLTAGE_1,
    BMV_CURRENT,
    BMV_CONSUMPTION,
    BMV_SOC,
    BMV_TIME_TO_GO,
    BMV_ALARM,
    BMV_RELAY,
    BMV_ALARM_REASON,
    BMV_FIRMWARE,
    BMV_MONITOR_MODE,
    BMV_TEMPERATURE,
    BMV_BMV};

class VEDirectObject
{
public:
    VEDirectObject(const VEDirectValueDefinition *definition, int len);
    ~VEDirectObject();

    void load_VEDirect_key_value(const char *line, unsigned long time);

    int get_number_value(int &value, const VEDirectValueDefinition& def) { return get_number_value(value, def.veIndex); }
    int get_number_value(double &value, double precision, const VEDirectValueDefinition& def) { return get_number_value(value, precision, def.veIndex); }
    int get_boolean_value(bool &value, const VEDirectValueDefinition& def) { return get_boolean_value(value, def.veIndex); }
    int get_string_value(char *value, const VEDirectValueDefinition& def) { return get_string_value(value, def.veIndex); }
    unsigned long get_last_timestamp(const VEDirectValueDefinition& def) { return get_last_timestamp(def.veIndex); }

    int get_number_value(int &value, unsigned int index);
    int get_number_value(double &value, double precision, unsigned int index);
    int get_boolean_value(bool &value, unsigned int index);
    int get_string_value(char *value, unsigned int index);
    unsigned long get_last_timestamp(unsigned int index);

    void reset();

    bool is_valid();

    void print();

private:
    int n_fields;
    int *i_values;
    char** s_values;
    unsigned long *last_time;
    int valid;
    const VEDirectValueDefinition *fields;
};

#endif