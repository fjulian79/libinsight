/*
 * libinsight, a libary to stream data in binary form to a host computer.
 *
 * Copyright (C) 2021 Julian Friedrich
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>. 
 *
 * You can file issues at https://github.com/fjulian79/libinsight/issues
 */

#include "insight/insight.hpp"
#include <string.h>

/**
 * @brief Defines the size of the data buffer used to transmit data to the host.
 * 
 * First byte is the header, followed the payload size and finally the payload 
 * itself. As we dont know what payload type will be used we assume 8 bytes per 
 * value.
 */
#define INSIGHT_DATABUFFERSIZ   (2 + (INSIGHT_NUMVALUES*8))

/**
 * @brief As only one byte is used to specify the payload site it is limited to
 * UINT8_8_MAX.
 */
#if (INSIGHT_NUMVALUES*8) > UINT8_MAX
#error "ERROR: Max Data buffer size violated, reduce INSIGHT_NUMVALUES!"
#endif

/**
 * @brief The used control characters.
 * 
 */
const struct
{
    const char SOH = 0x01;  /** Start of Header */
    const char STX = 0x02;  /** Start of text (data only) */
    const char ETX = 0x03;  /** End of text (data or header) */
    const char EOT = 0x04;  /** End of transmission */
    const char ESC = 0x1b;  /** Escape for all above */

}ctrl;

/**
 * @brief Defines the data to know per supported data type.
 */
typedef struct {

    size_t siz;         /** what sizeof(...) tells us */
    const char *hdr;    /** the string to use in the header */

} PayloadSpec_t;

/**
 * @brief Specifies the supported payload data types.
 */
const PayloadSpec_t PayloadSpec[11] = 
{
        {sizeof(bool),      "b"},
        {sizeof(uint8_t),   "u8"},
        {sizeof(uint16_t),  "u16"},
        {sizeof(uint32_t),  "u32"},
        {sizeof(uint64_t),  "u64"},
        {sizeof(int8_t),    "i8"},
        {sizeof(int16_t),   "i16"},
        {sizeof(int32_t),   "i32"},
        {sizeof(int64_t),   "i64"},
        {sizeof(float),     "f"},
        {sizeof(double),    "d"}
};

Insight::Insight() :
      Enabled(false)
    , Pause(false)
    , LastTick(0)
    , Period(INSIGHT_TASKPERIOD_MS)
{
    reset();
    setStream(&Serial);
}

void Insight::reset(void)
{
    memset(NameBuffer, 0, sizeof(NameBuffer));
    NameBufferPos = 0;
    
    memset(Payload, 0, sizeof(Payload));
    PayloadIdx = 0;

    PayloadSize = 2;
}

void Insight::setStream(Stream *pIoStr)
{
    pStream = pIoStr;
}

void Insight::setPeriod(uint32_t millis)
{
    Period = millis;
}

uint32_t Insight::getPeriod(void)
{
    return Period;
}

bool Insight::enable(bool state, bool sync)
{
    if (Enabled == state)
    {
        return true;
    }

    if (state && !Enabled)
    {
        if (PayloadIdx == 0)
        {
            /* Shall be enabled but no payload defined, can't do that. */
            return false;
        }

        pStream->write(ctrl.SOH);
        pStream->printf(INSIGHT_BINARYINFO_FMT);
        pStream->printf("%s", NameBuffer);

        for (uint8_t i = 0; i < PayloadIdx; i++)
        {
            pStream->printf("%s;", PayloadSpec[Payload[i].type].hdr);    
        }
        
        pStream->write(ctrl.ETX);

        /* If sync is requested manipulate the LastTick value to cause the task
         * function in it'S next call to become active. */
        if(sync)
        {
            LastTick -= 2*Period;
        }
        
        Enabled = true;
    }
    else if (!state && Enabled)
    {
        pStream->write(ctrl.EOT);
        Enabled = false;
    }

    return true;
}

bool Insight::isEnabled(void)
{
    return Enabled;
}

void Insight::pause(bool state, bool sync)
{
    Pause = state;

    /* If sync is requested manipulate the LastTick value to cause the task
     * function in it'S next call to become active. */
    if(sync)
    {
        LastTick -= 2*Period;
    }
}

bool Insight::isPaused(void)
{
    return Pause;
}

bool Insight::add(bool *ptr, const char *str)
{
    return add(ptr, dataType_bool, str);
}

bool Insight::add(uint8_t *ptr, const char *str)
{
    return add(ptr, dataType_uint_8, str);
}

bool Insight::add(uint16_t *ptr, const char *str)
{
    return add(ptr, dataType_uint_16, str);
}

bool Insight::add(uint32_t *ptr, const char *str)
{
    return add(ptr, dataType_uint_32, str);
}

bool Insight::add(uint64_t *ptr, const char *str)
{
    return add(ptr, dataType_uint_64, str);
}

bool Insight::add(int8_t *ptr, const char *str)
{
    return add(ptr, dataType_int_8, str);
}

bool Insight::add(int16_t *ptr, const char *str)
{
    return add(ptr, dataType_int_16, str);
}

bool Insight::add(int32_t *ptr, const char *str)
{
    return add(ptr, dataType_int_32, str);
}

bool Insight::add(int64_t *ptr, const char *str)
{
    return add(ptr, dataType_int_64, str);
}

bool Insight::add(float *ptr, const char *str)
{
    return add(ptr, dataType_float, str);
}

bool Insight::add(double *ptr, const char *str)
{
    return add(ptr, dataType_double, str);
}

bool Insight::add(void *ptr, dataTypes_t type, const char *name)
{
    /* While enabled, internal data has to be locked as it is used while 
     * transmitting data. */
    if (Enabled || (PayloadIdx == INSIGHT_NUMVALUES))
    {
        return false;
    }

    size_t size = INSIGHT_NAMEBUFFERSIZ - NameBufferPos;
    if (size == 0)
    {
        /* No place left in the buffer; */
        return false;
    }

    if (PayloadSize + PayloadSpec[type].siz > INSIGHT_DATABUFFERSIZ)
    {
        return false;
    }

    int written = snprintf((char*) &NameBuffer[NameBufferPos], 
            size, "%s;", name);

    /* Negative return values are severe errors, it written is larger then size 
     * then the name does not fit into the buffer. */
    if ((written > 0) && (written < size))
    {
        NameBufferPos+=written;
        Payload[PayloadIdx].ptr = ptr;
        Payload[PayloadIdx].type = type;
        PayloadSize += PayloadSpec[type].siz;
        PayloadIdx++;
    }
    else
    {
        /* No success while writing to the name buffer, make sure it is properly
         * terminated. */
        NameBuffer[NameBufferPos] = 0;
        return false;
    }

    return true;
}

bool Insight::transmit(void)
{
    uint8_t buffer[32];
    uint8_t idx = 0;

    if (!Enabled)
    {
        return false;
    }

    /* Data transmission shall be fast as possible. As consequence I have 
     * decided to:
     *
     * # Don't use explicit frame termination as this will result in the need of
     *   escaping control characters.
     *
     * # Use a buffer to collect the data and dont transmit each value on it's 
     *   own. At least my measurements have shown that this is faster.
     */
    buffer[idx++] = ctrl.STX;
    buffer[idx++] = PayloadSize-2;

    for (uint8_t i = 0; i < PayloadIdx; i++)
    {
        size_t siz = PayloadSpec[Payload[i].type].siz;
        memcpy(&buffer[idx], (uint8_t*)Payload[i].ptr, siz);
        idx+= siz;
    }

    pStream->write(buffer, idx);

    return true;
}

void Insight::task(uint32_t millis)
{
    if(!Enabled || Pause)
    {
        return;
    }

    if (millis - LastTick > Period)
    {
        transmit();
        LastTick = millis;
    }
}