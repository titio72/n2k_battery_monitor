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

static const unsigned int BMV_N_FIELDS = 13;
static const VEDirectValueDefinition PID(VE_NUMBER, "PID", 0);
static const VEDirectValueDefinition VOLTAGE(VE_NUMBER, "V", 1, "mV");
static const VEDirectValueDefinition VOLTAGE_1(VE_NUMBER, "VS", 2, "mV");
static const VEDirectValueDefinition CURRENT(VE_NUMBER, "I", 3, "mA");
static const VEDirectValueDefinition CONSUMPTION(VE_NUMBER, "CE", 4, "mAh");
static const VEDirectValueDefinition SOC(VE_NUMBER, "SOC", 5, "1/1000");
static const VEDirectValueDefinition TIME_TO_GO(VE_NUMBER, "TTG", 6, "Minutes");
static const VEDirectValueDefinition ALARM(VE_BOOLEAN, "Alarm", 7);
static const VEDirectValueDefinition RELAY(VE_BOOLEAN, "Relay", 8);
static const VEDirectValueDefinition ALARM_REASON(VE_NUMBER, "AR", 9, "Enum");
static const VEDirectValueDefinition FIRMWARE(VE_STRING, "FW", 10);
static const VEDirectValueDefinition MONITOR_MODE(VE_NUMBER, "MON", 11, "Enum");
static const VEDirectValueDefinition TEMPERATURE(VE_NUMBER, "T", 12, "C");
static const VEDirectValueDefinition BMV_FIELDS[BMV_N_FIELDS] = {
    PID, VOLTAGE, VOLTAGE_1, CURRENT, CONSUMPTION, SOC, TIME_TO_GO, ALARM, RELAY, ALARM_REASON, FIRMWARE, MONITOR_MODE, TEMPERATURE};

typedef VEDirectValueDefinition *prtVEDirectValueDefinition;

class VEDirectObject
{
public:
    VEDirectObject(const VEDirectValueDefinition *definition, int len);
    ~VEDirectObject();

    void load_VEDirect_key_value(const char *line, unsigned long time);
    int get_number_value(int &value, unsigned int index);
    int get_number_value(double &value, double precision, unsigned int index);
    int get_boolean_value(bool &value, unsigned int index);
    int get_string_value(char *value, unsigned int index);
    unsigned long get_last_timestamp(unsigned int index);

    void reset();

    bool is_valid();

private:
    int n_fields;
    int *i_values;
    unsigned long *last_time;
    int valid;
    const VEDirectValueDefinition *fields;
};

#endif