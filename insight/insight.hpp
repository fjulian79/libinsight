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

#ifndef INSIGHT_HPP_
#define INSIGHT_HPP_

#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>

#if __has_include ("insight_config.hpp")
#include "insight_config.hpp"
#endif

#include "insight/config.hpp"

/**
 * @brief This enum is used to define data types. The interger values 
 * are used to access const data arrays defined by the implementation.
 */
typedef enum {

    dataType_bool    = 0,
    dataType_uint_8  = 1,
    dataType_uint_16 = 2,
    dataType_uint_32 = 3,
    dataType_uint_64 = 4,
    dataType_int_8   = 5,
    dataType_int_16  = 6,
    dataType_int_32  = 7,
    dataType_int_64  = 8,
    dataType_float   = 9,
    dataType_double  = 10

}dataTypes_t; 

class Insight
{
    public:
    
        /**
         * @brief Construct a new Insight object
         */
        Insight();

        /**
         * @brief Rests stream related data.
         * 
         * Call this function when you want to start over on the fly be removing 
         * all previously added variables, and new ones and start streaming 
         * data.
         * 
         * This function has not to be called when just starting a previously 
         * stopped transmission.
         */
        void reset(void);

        /**
         * @brief Used to set the Stream to operate on. 
         * 
         * The Default is Serial 
         * 
         * @param pIoStr The Stream to use.
         */
        void setStream(Stream *pIoStr);

        /**
         * @brief Used to set the period of the task function.
         * 
         * @param millis The period in milli seconds.
         */
        void setPeriod(uint32_t millis);

        /**
         * @brief Tells the currently configured insight task period
         * 
         * @return Task period in ms.
         */
        uint32_t getPeriod(void);

        /**
         * @brief Used to enable or disable the data transmission.
         * 
         * @param state True to allow data transmission, in this case the header 
         *              will be transmitted right away. From this moment on the 
         *              internal data structures are locked and no further 
         *              variables can be added. So calls of add(...) will fail 
         *              while enabled. Also, when the task is scheduled next 
         *              time it will start transmitting data.
         *          
         *              False to prevent data tansmission.
         * 
         * @param sync  If set to true the task function will schedule data 
         *              transmission when called next time and will not wait 
         *              until it's next tick. From this time on it will 
         *              transmitt data in the given interval.
         * 
         * @return true in case of success. 
         * @return false if the transmission can not be started.
         */
        bool enable(bool state, bool sync=false);

        /**
         * @brief Tells if data transmission is taking place or not.
         * 
         * Basically it just tells you if there has been a call of enable(true).
         * This starts the transmission if the provided task function is used. 
         * If you don't use the task you have to call transmit() on your own 
         * somehow and there will be no tranmission by the lib.
         * 
         * @return true data transmitted is enabled
         * @return false if no data is not enabled.
         */
        bool isEnabled(void);

        /**
         * @brief Allows to pause a active data transmission.
         * 
         * Pause can be enabled at any time. If enabled the task function will 
         * NOT transmit any data no mather if the stream is enabled or not. 
         * 
         * Calling enable(...) has no effect on the pause state.
         * 
         * HENCE: Calling pause(false) will not trigger transmission of the 
         *        header data. 
         * 
         * @param state the new pause state.
         * 
         * @param sync  If set to true the task function will schedule data 
         *              transmission when called next time and will not wait 
         *              until it's next tick. From this time on it will 
         *              transmitt data in the given interval.
         */
        void pause(bool state, bool sync=false);

        /**
         * @brief Tells the current pause state.
         * 
         * @return true data transmission is paused 
         * @return false data transmission is enabled.
         */
        bool isPaused(void);

        /**
         * @brief Used to add a variable to the data stream.
         * 
         * @param ptr Point to your variable of type bool.
         * @param str A string to identify the variable later on.
         *
         * @return true if it has been added successfully.
         * @return false if can't be added. Either because data transmission is 
         *         possible (see above enable(...)) or the internal data 
         *         structures can't take more data.
         */
        bool add(bool *ptr, const char *str);

        /**
         * @brief Used to add a variable to the data stream.
         * 
         * @param ptr Point to your variable of type uint8_t.
         * @param str A string to identify the variable later on.
         *
         * @return true if it has been added successfully.
         * @return false if can't be added. Either because data transmission is 
         *         possible (see above enable(...)) or the internal data 
         *         structures can't take more data.
         */
        bool add(uint8_t *ptr, const char *str);

        /**
         * @brief Used to add a variable to the data stream.
         * 
         * @param ptr Point to your variable of type uint16_t.
         * @param str A string to identify the variable later on.
         *
         * @return true if it has been added successfully.
         * @return false if can't be added. Either because data transmission is 
         *         possible (see above enable(...)) or the internal data 
         *         structures can't take more data.
         */
        bool add(uint16_t *ptr, const char *str);

        /**
         * @brief Used to add a variable to the data stream.
         * 
         * @param ptr Point to your variable of type uint32_t.
         * @param str A string to identify the variable later on.
         *
         * @return true if it has been added successfully.
         * @return false if can't be added. Either because data transmission is 
         *         possible (see above enable(...)) or the internal data 
         *         structures can't take more data.
         */
        bool add(uint32_t *ptr, const char *str);

        /**
         * @brief Used to add a variable to the data stream.
         * 
         * @param ptr Point to your variable of type uint64_t.
         * @param str A string to identify the variable later on.
         *
         * @return true if it has been added successfully.
         * @return false if can't be added. Either because data transmission is 
         *         possible (see above enable(...)) or the internal data 
         *         structures can't take more data.
         */
        bool add(uint64_t *ptr, const char *str);

        /**
         * @brief Used to add a variable to the data stream.
         * 
         * @param ptr Point to your variable of type int8_t.
         * @param str A string to identify the variable later on.
         *
         * @return true if it has been added successfully.
         * @return false if can't be added. Either because data transmission is 
         *         possible (see above enable(...)) or the internal data 
         *         structures can't take more data.
         */
        bool add(int8_t *ptr, const char *str);

        /**
         * @brief Used to add a variable to the data stream.
         * 
         * @param ptr Point to your variable of type int16_t.
         * @param str A string to identify the variable later on.
         *
         * @return true if it has been added successfully.
         * @return false if can't be added. Either because data transmission is 
         *         possible (see above enable(...)) or the internal data 
         *         structures can't take more data.
         */
        bool add(int16_t *ptr, const char *str);

        /**
         * @brief Used to add a variable to the data stream.
         * 
         * @param ptr Point to your variable of type int32_t.
         * @param str A string to identify the variable later on.
         *
         * @return true if it has been added successfully.
         * @return false if can't be added. Either because data transmission is 
         *         possible (see above enable(...)) or the internal data 
         *         structures can't take more data.
         */
        bool add(int32_t *ptr, const char *str);

        /**
         * @brief Used to add a variable to the data stream.
         * 
         * @param ptr Point to your variable of type int64_t.
         * @param str A string to identify the variable later on.
         *
         * @return true if it has been added successfully.
         * @return false if can't be added. Either because data transmission is 
         *         possible (see above enable(...)) or the internal data 
         *         structures can't take more data.
         */
        bool add(int64_t *ptr, const char *str);

        /**
         * @brief Used to add a variable to the data stream.
         * 
         * @param ptr Point to your variable of type float.
         * @param str A string to identify the variable later on.
         *
         * @return true if it has been added successfully.
         * @return false if can't be added. Either because data transmission is 
         *         possible (see above enable(...)) or the internal data 
         *         structures can't take more data.
         */
        bool add(float *ptr, const char *str);

        /**
         * @brief Used to add a variable to the data stream.
         * 
         * @param ptr Point to your variable of type double.
         * @param str A string to identify the variable later on.
         *
         * @return true if it has been added successfully.
         * @return false if can't be added. Either because data transmission is 
         *         possible (see above enable(...)) or the internal data 
         *         structures can't take more data.
         */
        bool add(double *ptr, const char *str);

        /**
         * @brief The function implementing the add command.
         * 
         * The public add variants just hand over the correct data for their
         * particular data type.
         * 
         * @param ptr The pointer to the users variable.
         * @param type The type specification. 
         * @param name The name of the variable.
         * @return true in case of success.
         * @return false in case of a error.
         */
        bool add(void *ptr, dataTypes_t type, const char *name);

        /**
         * @brief Used to collect the data added to the data transmission and 
         * transmitt a single frame to the host. 
         * 
         * This function is called by the task(...) function with the configured 
         * period.
         * 
         * @return true in case of success. 
         * @return false if the transmission has not been enabled before. 
         */
        bool transmit(void);

        /**
         * @brief The libaries Arduino style task function.
         * 
         * Call this in your main loop as fast as possible, it will transmit 
         * data on it's own in the set interval. See setPeriod(...)
         * 
         * @param millis The current wall clock in ms.
         */
        void task(uint32_t millis);

    private:

        /**
         * @brief The internal enabled state.
         */
        bool Enabled;

        /**
         * @brief The internal pause state.
         */
        bool Pause;

        /**
         * @brief The last tick of the task function.
         */
        uint32_t LastTick;

        /**
         * @brief The tasks period in ms.
         */
        uint32_t Period;

        /**
         * @brief The stream to use.
         */
        Stream *pStream;

        /**
         * @brief The buffer taking the users variable names.
         * 
         * Filled by add(...), cleared by reset(...), transmitted by enable(...) 
         */
        uint8_t NameBuffer[INSIGHT_NAMEBUFFERSIZ];

        /**
         * @brief The current position in the name buffer.
         */
        uint8_t NameBufferPos;

        /**
         * @brief The Array holding the data needed for transmitting data.
         */
        struct {
            
            void            *ptr;   /** The point tot the data */
            dataTypes_t     type;   /** The type of the data */

        } Payload[INSIGHT_NUMVALUES];

        /**
         * @brief The number of used payload elements.
         * 
         */
        uint8_t PayloadIdx;

        /**
         * @brief The number of payload bytes to transmit.
         */
        uint8_t PayloadSize;
};

#endif /* INSIGHT_HPP_ */
