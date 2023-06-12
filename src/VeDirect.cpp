#include "VeDirect.h"
#include "Utils.h"
#include "Log.h"
#include <stdio.h>

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
    Reset();
}

VEDirectObject::~VEDirectObject()
{
    delete i_values;
    delete last_time;
}

void VEDirectObject::LoadVEDirectKeyValue(const char *line, unsigned long time)
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

int VEDirectObject::GetNumberValue(int &value, unsigned int index)
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

int VEDirectObject::GetNumberValue(double &value, double precision, unsigned int index)
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

int VEDirectObject::GetBooleanValue(bool &value, unsigned int index)
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

unsigned long VEDirectObject::GetLastTimestamp(unsigned int index)
{
    if (index > n_fields)
        return 0;
    return last_time[index];
}

int VEDirectObject::GetStringValue(char *value, unsigned int index)
{
    return 0; // unsupported for now
}

void VEDirectObject::Reset()
{
    for (int i = 0; i < n_fields; i++)
        last_time[i] = 0;
    for (int i = 0; i < n_fields; i++)
        i_values[i] = 0;
    valid = 0;
}

bool VEDirectObject::IsValid()
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