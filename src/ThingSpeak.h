/*
  ThingSpeak(TM) Communication Library For Particle

  Enables Particle hardware to write or read data to or from ThingSpeak,
  an open data platform for the Internet of Things with MATLAB analytics and visualization. 

  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize and analyze live data streams in the cloud.
  
  Copyright 2020-2025, The MathWorks, Inc.
 
  See the accompanying license file for licensing information.
*/

//#define PRINT_DEBUG_MESSAGES
//#define PRINT_HTTP
 
#ifndef ThingSpeak_h
    #define ThingSpeak_h

    #define TS_VER "1.6.0"


    // Create platform defines for Particle devices
    #if PLATFORM_ID == 0
        #define PARTICLE_CORE
    #elif PLATFORM_ID == 6
        #define PARTICLE_PHOTON
        #define PARTICLE_PHOTONELECTRON
    #elif PLATFORM_ID == 8
        #define PARTICLE_P1
        #define PARTICLE_PHOTONELECTRON
    #elif PLATFORM_ID == 10
        #define PARTICLE_ELECTRON
        #define PARTICLE_PHOTONELECTRON
    #elif PLATFORM_ID == 12
        #define PARTICLE_ARGON
        #define PARTICLE_PHOTONELECTRON
    #elif PLATFORM_ID == 13
        #define PARTICLE_BORON
        #define PARTICLE_PHOTONELECTRON
    #elif PLATFORM_ID == 23
        #define PARTICLE_BSOM
    #elif PLATFORM_ID == 25
        #define PARTICLE_B5SOM
    #elif PLATFORM_ID == 14
        #error TCP connection are not supported on mesh nodes (Xenon), only mesh gateways (Argon, Boron)
    #else
        #error Only Core/Photon/Electron/P1/Argon/Boron are supported.
    #endif


    #include "math.h"
    #include "application.h"
    #ifdef PARTICLE_PHOTONELECTRON
        extern char* dtoa(double val, unsigned char prec, char *sout);
        // On spark photon, There is no itoa, so map to ltoa.
        #include "string_convert.h"
        #define itoa ltoa
    #else
        // On spark core, a long and an int are equivalent, and so there's no "ltoa" function defined.  Map it to itoa.
        extern char * itoa(int a, char* buffer, unsigned char radix);
        #define ltoa itoa
        extern char *dtostrf (double val, signed char width, unsigned char prec, char *sout);
    #endif


    #define THINGSPEAK_URL "api.thingspeak.com"
    #define THINGSPEAK_PORT_NUMBER 80


    #ifdef PARTICLE_CORE
        #define TS_USER_AGENT "tslib-arduino/" TS_VER " (particle core)"
    #elif defined(PARTICLE_PHOTON)
        #define TS_USER_AGENT "tslib-arduino/" TS_VER " (particle photon)"
    #elif defined(PARTICLE_ELECTRON)
        #define TS_USER_AGENT "tslib-arduino/" TS_VER " (particle electron)"
    #elif defined(PARTICLE_P1)
        #define TS_USER_AGENT "tslib-arduino/" TS_VER " (particle p1)"
    #elif defined(PARTICLE_ARGON)
        #define TS_USER_AGENT "tslib-arduino/" TS_VER " (particle argon)"
    #elif defined(PARTICLE_BORON)
        #define TS_USER_AGENT "tslib-arduino/" TS_VER " (particle boron)"
    #else
        #define TS_USER_AGENT "tslib-arduino/" TS_VER " (particle unknown)"
    #endif
    #define SPARK_PUBLISH_TTL 60 // Spark "time to live" for published messages
    #define SPARK_PUBLISH_TOPIC "thingspeak-debug"


    #define FIELDNUM_MIN 1
    #define FIELDNUM_MAX 8
    #define FIELDLENGTH_MAX 255  // Max length for a field in ThingSpeak is 255 bytes (UTF-8)

    #define TIMEOUT_MS_SERVERRESPONSE 5000  // Wait up to five seconds for server to respond

    #define TS_OK_SUCCESS              200     // OK / Success
    #define TS_ERR_BADAPIKEY           400     // Incorrect API key (or invalid ThingSpeak server address)
    #define TS_ERR_BADURL              404     // Incorrect API key (or invalid ThingSpeak server address)
    #define TS_ERR_OUT_OF_RANGE        -101    // Value is out of range or string is too long (> 255 bytes)
    #define TS_ERR_INVALID_FIELD_NUM   -201    // Invalid field number specified
    #define TS_ERR_SETFIELD_NOT_CALLED -210    // setField() was not called before writeFields()
    #define TS_ERR_CONNECT_FAILED      -301    // Failed to connect to ThingSpeak
    #define TS_ERR_UNEXPECTED_FAIL     -302    // Unexpected failure during write to ThingSpeak
    #define TS_ERR_BAD_RESPONSE        -303    // Unable to parse response
    #define TS_ERR_TIMEOUT             -304    // Timeout waiting for server to respond
    #define TS_ERR_NOT_INSERTED        -401    // Point was not inserted (most probable cause is the rate limit of once every 15 seconds)

    
    // variables to store the values from the readMultipleFields functionality
    typedef struct feedRecord
    {
        String nextReadField[8];
        String nextReadStatus;
        String nextReadLatitude;
        String nextReadLongitude;
        String nextReadElevation;
        String nextReadCreatedAt;
    }feed;

    
    // Enables Particle hardware to write or read data to or from ThingSpeak, an open data platform for the Internet of Things with MATLAB analytics and visualization.
    class ThingSpeakClass
    {
      public:
        ThingSpeakClass()
        {
            resetWriteFields();
            this->lastReadStatus = TS_OK_SUCCESS;
        };

        /*
        Function: begin
        
        Summary:
        Initializes the ThingSpeak library and network settings using the ThingSpeak.com service.
        
        Parameters:
        client - TCPClient created earlier in the sketch
        
        Returns:
        Always returns true
        
        Notes:
        This does not validate the information passed in, or generate any calls to ThingSpeak.
        */
        bool begin(Client & client)
        {
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "ts::tsBegin", SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            this->setClient(&client);
            this->setPort(THINGSPEAK_PORT_NUMBER);
            resetWriteFields();
            this->lastReadStatus = TS_OK_SUCCESS;
            return true;
        }
        
        
        /*
        Function: writeField
        
        Summary:
        Write an integer value to a single field in a ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to write to.
        value - Integer value (from -32,768 to 32,767) to write.
        writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Returns:
        HTTP status code of 200 if successful.
        
        Notes:
        See getLastReadStatus() for other possible return values.
        */
        int writeField(unsigned long channelNumber, unsigned int field, int value, const char * writeAPIKey)
        {
            // On Spark, int and long are the same, so map to the long version
            return writeField(channelNumber, field, (long)value, writeAPIKey);
        }
        

        /*
        Function: writeField
        
        Summary:
        Write a long value to a single field in a ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to write to.
        value - Long value (from -2,147,483,648 to 2,147,483,647) to write.
        writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Returns:
        HTTP status code of 200 if successful.
        
        Notes:
        See getLastReadStatus() for other possible return values.
        */
        int writeField(unsigned long channelNumber, unsigned int field, long value, const char * writeAPIKey)
        {
            char valueString[15];  // long range is -2147483648 to 2147483647, so 12 bytes including terminator
            ltoa(value, valueString, 10);
            return writeField(channelNumber, field, valueString, writeAPIKey);
        }

        
        /*
        Function: writeField
        
        Summary:
        Write a floating point value to a single field in a ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to write to.
        value - Floating point value (from -999999000000 to 999999000000) to write.  If you need more accuracy, or a wider range, you should format the number using <tt>dtostrf</tt> and writeField().
        writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Returns:
        HTTP status code of 200 if successful.
        
        Notes:
        See getLastReadStatus() for other possible return values.
        */
        int writeField(unsigned long channelNumber, unsigned int field, float value, const char * writeAPIKey)
        {
            #ifdef PRINT_DEBUG_MESSAGES
            Particle.publish(SPARK_PUBLISH_TOPIC, "ts::writeField (channelNumber: " + String(channelNumber) + " writeAPIKey: " + String(writeAPIKey) + " field: " + String(field) + " value: " + String(value,5) + ")" , SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            char valueString[20]; // range is -999999000000.00000 to 999999000000.00000, so 19 + 1 for the terminator
            int status = convertFloatToChar(value, valueString);
            if(status != TS_OK_SUCCESS) return status;

            return writeField(channelNumber, field, valueString, writeAPIKey);
        }
        

        /*
        Function: writeField
        
        Summary:
        Write a string to a single field in a ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to write to.
        value - String to write (UTF8 string).  ThingSpeak limits this field to 255 bytes.
        writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Returns:
        HTTP status code of 200 if successful.
        
        Notes:
        See getLastReadStatus() for other possible return values.
        */
        int writeField(unsigned long channelNumber, unsigned int field, String value, const char * writeAPIKey)
        {
            // Invalid field number specified
            if(field < FIELDNUM_MIN || field > FIELDNUM_MAX) return TS_ERR_INVALID_FIELD_NUM;
            // Max # bytes for ThingSpeak field is 255
            if(value.length() > FIELDLENGTH_MAX) return TS_ERR_OUT_OF_RANGE;
            
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "writeField (" + String(channelNumber) + ", " + String(writeAPIKey) + ", " + String(field) + ", " + escapeUrl(value) + ")", SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            String postMessage = String("field") + String(field) + "=" + escapeUrl(value);
            return writeRaw(channelNumber, postMessage, writeAPIKey);
        }
        

        /*
        Function: setField
        
        Summary:
        Set the value of a single field that will be part of a multi-field update.
        
        Parameters:
        field - Field number (1-8) within the channel to set.
        value - Integer value (from -32,768 to 32,767) to set.
        
        Returns:
        Code of 200 if successful.
        Code of -101 if value is out of range or string is too long (> 255 bytes)
        */
        int setField(unsigned int field, int value)
        {
            // On Spark, int and long are the same, so map to the long version
            return setField(field, (long)value);
        }
        

        /*
        Function: setField
        
        Summary:
        Set the value of a single field that will be part of a multi-field update.
        
        Parameters:
        field - Field number (1-8) within the channel to set.
        value - Long value (from -2,147,483,648 to 2,147,483,647) to write.
        
        Returns:
        Code of 200 if successful.
        Code of -101 if value is out of range or string is too long (> 255 bytes)
        */
        int setField(unsigned int field, long value)
        {
            char valueString[15];  // long range is -2147483648 to 2147483647, so 12 bytes including terminator
            ltoa(value, valueString, 10);
            return setField(field, valueString);
        }
        

        /*
        Function: setField
        
        Summary:
        Set the value of a single field that will be part of a multi-field update.
        
        Parameters:
        field - Field number (1-8) within the channel to set.
        value - Floating point value (from -999999000000 to 999999000000) to write.  If you need more accuracy, or a wider range, you should format the number yourself (using <tt>dtostrf</tt>) and setField() using the resulting string.
        
        Returns:
        Code of 200 if successful.
        Code of -101 if value is out of range or string is too long (> 255 bytes)
        */
        int setField(unsigned int field, float value)
        {
            char valueString[20]; // range is -999999000000.00000 to 999999000000.00000, so 19 + 1 for the terminator
            int status = convertFloatToChar(value, valueString);
            if(status != TS_OK_SUCCESS) return status;

            return setField(field, valueString);
        }
        

        /*
        Function: setField
        
        Summary:
        Set the value of a single field that will be part of a multi-field update.
        
        Parameters:
        field - Field number (1-8) within the channel to set.
        value - String to write (UTF8).  ThingSpeak limits this to 255 bytes.
        
        Returns:
        Code of 200 if successful.
        Code of -101 if value is out of range or string is too long (> 255 bytes)
        */
        int setField(unsigned int field, String value)
        {
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "setField " + String(field) + " to " + String(value), SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            if(field < FIELDNUM_MIN || field > FIELDNUM_MAX) return TS_ERR_INVALID_FIELD_NUM;
            // Max # bytes for ThingSpeak field is 255 (UTF-8)
            if(value.length() > FIELDLENGTH_MAX) return TS_ERR_OUT_OF_RANGE;
            this->nextWriteField[field - 1] = value;
            return TS_OK_SUCCESS;
        }
        

        /*
        Function: setLatitude
        
        Summary:
        Set the latitude of a multi-field update.
        
        Parameters:
        latitude - Latitude of the measurement as a floating point value (degrees N, use negative values for degrees S)
        
        Returns:
        Always return 200
        
        Notes:
        To record latitude, longitude and elevation of a write, call setField() for each of the fields you want to write. Then setLatitude(), setLongitude(), setElevation() and then call writeFields()
        */
        int setLatitude(float latitude)
        {
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "ts::setLatitude(latitude: " + String(latitude,3) + "\")" , SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            this->nextWriteLatitude = latitude;
            return TS_OK_SUCCESS;
        }
        

        /*
        Function: setLongitude
        
        Summary:
        Set the longitude of a multi-field update.
        
        Parameters:
        longitude - Longitude of the measurement as a floating point value (degrees E, use negative values for degrees W)
        
        Returns:
        Always return 200
        
        Notes:
        To record latitude, longitude and elevation of a write, call setField() for each of the fields you want to write. Then setLatitude(), setLongitude(), setElevation() and then call writeFields()
        */
        int setLongitude(float longitude)
        {
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "ts::setLongitude(longitude: " + String(longitude,3) + "\")" , SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            this->nextWriteLongitude = longitude;
            return TS_OK_SUCCESS;
        }
        

        /*
        Function: setElevation
        
        Summary:
        Set the elevation of a multi-field update.
        
        Parameters:
        elevation - Elevation of the measurement as a floating point value (meters above sea level)
        
        Returns:
        Always return 200
        
        Notes:
        To record latitude, longitude and elevation of a write, call setField() for each of the fields you want to write. Then setLatitude(), setLongitude(), setElevation() and then call writeFields()
        */
        int setElevation(float elevation)
        {
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "ts::setElevation(elevation: " + String(elevation,3) + "\")" , SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            this->nextWriteElevation = elevation;
            return TS_OK_SUCCESS;
        }
        
        
        /*
        Function: setStatus
        
        Summary:
        Set the status field of a multi-field update.
        
        Parameters:
        status - String to write (UTF8).  ThingSpeak limits this to 255 bytes.
        
        Returns:
        Code of 200 if successful.
        Code of -101 if string is too long (> 255 bytes)
        
        Notes:
        To record a status message on a write, call setStatus() then call writeFields().
        Use status to provide additonal details when writing a channel update.
        */
        int setStatus(String status)
        {
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "ts::setStatus(status: " + status + "\")" , SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            // Max # bytes for ThingSpeak field is 255 (UTF-8)
            if(status.length() > FIELDLENGTH_MAX) return TS_ERR_OUT_OF_RANGE;
            this->nextWriteStatus = status;
            return TS_OK_SUCCESS;
        }       
       
        
        /*
        Function: setCreatedAt
        
        Summary:
        Set the created-at date of a multi-field update.
        
        Parameters:
        createdAt - Desired timestamp to be included with the channel update as a String.  The timestamp string must be in the ISO 8601 format. Example "2017-01-12 13:22:54"
        
        Returns:
        Code of 200 if successful.
        Code of -101 if string is too long (> 255 bytes)
        
        Notes:
        Timezones can be set using the timezone hour offset parameter. For example, a timestamp for Eastern Standard Time is: "2017-01-12 13:22:54-05".
        If no timezone hour offset parameter is used, UTC time is assumed.
        */
        int setCreatedAt(String createdAt)
        {
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "ts::setCreatedAt(createdAt: " + createdAt + "\")" , SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            
            // the ISO 8601 format is too complicated to check for valid timestamps here
            // we'll need to reply on the api to tell us if there is a problem
            // Max # bytes for ThingSpeak field is 255 (UTF-8)
            if(createdAt.length() > FIELDLENGTH_MAX) return TS_ERR_OUT_OF_RANGE;
            this->nextWriteCreatedAt = createdAt;
            
            return TS_OK_SUCCESS;
        }
        
        
        /*
        Function: writeFields
        
        Summary:
        Write a multi-field update.
        
        Parameters:
        channelNumber - Channel number
        writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Returns:
        200 - successful.
        404 - Incorrect API key (or invalid ThingSpeak server address)
        -101 - Value is out of range or string is too long (> 255 characters)
        -201 - Invalid field number specified
        -210 - setField() was not called before writeFields()
        -301 - Failed to connect to ThingSpeak
        -302 - Unexpected failure during write to ThingSpeak
        -303 - Unable to parse response
        -304 - Timeout waiting for server to respond
        -401 - Point was not inserted (most probable cause is the rate limit of once every 15 seconds)
        
        
        Notes:
        Call setField(), setLatitude(), setLongitude(), setElevation() and/or setStatus() and then call writeFields()
        */
        int writeFields(unsigned long channelNumber, const char * writeAPIKey)
        {
            String postMessage = String("");
            bool fFirstItem = true;
            for(size_t iField = 0; iField < 8; iField++)
            {
                if(this->nextWriteField[iField].length() > 0)
                {
                    if(!fFirstItem)
                    {
                        postMessage = postMessage + String("&");
                    }
                    postMessage = postMessage + String("field") + String(iField + 1) + String("=") + escapeUrl(this->nextWriteField[iField]);
                    fFirstItem = false;
                    this->nextWriteField[iField] = "";
                }
            }

            if(!isnan(nextWriteLatitude))
            {
                if(!fFirstItem)
                {
                    postMessage = postMessage + String("&");
                }
                postMessage = postMessage + String("lat=") + String(this->nextWriteLatitude);
                fFirstItem = false;
                this->nextWriteLatitude = NAN;
            }

            if(!isnan(this->nextWriteLongitude))
            {
                if(!fFirstItem)
                {
                    postMessage = postMessage + String("&");
                }
                postMessage = postMessage + String("long=") + String(this->nextWriteLongitude);
                fFirstItem = false;
                this->nextWriteLongitude = NAN;
            }


            if(!isnan(this->nextWriteElevation))
            {
                if(!fFirstItem)
                {
                    postMessage = postMessage + String("&");
                }
                postMessage = postMessage + String("elevation=") + String(this->nextWriteElevation);
                fFirstItem = false;
                this->nextWriteElevation = NAN;
            }
            
            if(this->nextWriteStatus.length() > 0)
            {
                if(!fFirstItem)
                {
                    postMessage = postMessage + String("&");
                }
                postMessage = postMessage + String("status=") + escapeUrl(this->nextWriteStatus);
                fFirstItem = false;
                this->nextWriteStatus = "";
            }
            
            if(this->nextWriteCreatedAt.length() > 0)
            {
                if(!fFirstItem)
                {
                    postMessage = postMessage + String("&");
                }
                postMessage = postMessage + String("created_at=") + String(this->nextWriteCreatedAt);
                fFirstItem = false;
                this->nextWriteCreatedAt = "";
            }
            
            
            if(fFirstItem)
            {
                // setField was not called before writeFields
                return TS_ERR_SETFIELD_NOT_CALLED;
            }

            return writeRaw(channelNumber, postMessage, writeAPIKey);
        }

        
        /*
        Function: writeRaw
        
        Summary:
        Write a raw POST to a ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        postMessage - Raw URL to write to ThingSpeak as a string.  See the documentation at https://thingspeak.com/docs/channels#update_feed.
        writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Returns:
        200 - successful.
        404 - Incorrect API key (or invalid ThingSpeak server address)
        -101 - Value is out of range or string is too long (> 255 characters)
        -201 - Invalid field number specified
        -210 - setField() was not called before writeFields()
        -301 - Failed to connect to ThingSpeak
        -302 - Unexpected failure during write to ThingSpeak
        -303 - Unable to parse response
        -304 - Timeout waiting for server to respond
        -401 - Point was not inserted (most probable cause is the rate limit of once every 15 seconds)
        
        Notes:
        This is low level functionality that will not be required by most users.
        */
        int writeRaw(unsigned long channelNumber, String postMessage, const char * writeAPIKey)
        {
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "ts::writeRaw   (channelNumber: " + String(channelNumber) + " writeAPIKey: " + String(writeAPIKey) + " postMessage: \"" + postMessage + "\")" , SPARK_PUBLISH_TTL, PRIVATE);
            #endif

            if(!connectThingSpeak())
            {
                // Failed to connect to ThingSpeak
                return TS_ERR_CONNECT_FAILED;
            }

            postMessage = postMessage + String("&headers=false");

            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "Post " + postMessage, SPARK_PUBLISH_TTL, PRIVATE);
            #endif


            // Post data to thingspeak
            if(!this->client->print("POST /update HTTP/1.1\r\n")) return abortWriteRaw();
            if(!writeHTTPHeader(writeAPIKey)) return abortWriteRaw();
            if(!this->client->print("Content-Type: application/x-www-form-urlencoded\r\n")) return abortWriteRaw();
            if(!this->client->print("Content-Length: ")) return abortWriteRaw();
            if(!this->client->print(postMessage.length())) return abortWriteRaw();
            if(!this->client->print("\r\n\r\n")) return abortWriteRaw();
            if(!this->client->print(postMessage)) return abortWriteRaw();
      
            String entryIDText = String();
            int status = getHTTPResponse(entryIDText);
            if(status != TS_OK_SUCCESS)
            {
                client->stop();
                return status;
            }
            long entryID = entryIDText.toInt();

            #ifdef PRINT_DEBUG_MESSAGES
            Particle.publish(SPARK_PUBLISH_TOPIC, "               Entry ID \"" + entryIDText + "\" (" + String(entryID) + ")" , SPARK_PUBLISH_TTL, PRIVATE);
            #endif

            client->stop();
            
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "disconnected.", SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            if(entryID == 0)
            {
                // ThingSpeak did not accept the write
                status = TS_ERR_NOT_INSERTED;
            }
            return status;
        }
        
        
        /*
        Function: readStringField
        
        Summary:
        Read the latest string from a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to read from.
        readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Returns:
        Value read (UTF8 string), or empty string if there is an error.  Use getLastReadStatus() to get more specific information.
        */
        String readStringField(unsigned long channelNumber, unsigned int field, const char * readAPIKey)
        {
            if(field < FIELDNUM_MIN || field > FIELDNUM_MAX)
            {
                this->lastReadStatus = TS_ERR_INVALID_FIELD_NUM;
                return("");
            }
            #ifdef PRINT_DEBUG_MESSAGES
                
                if(NULL != readAPIKey)
                {
                    Particle.publish(SPARK_PUBLISH_TOPIC, "ts::readStringField(channelNumber: " + String(channelNumber) + " readAPIKey: " + String(readAPIKey) + " field: " + String(field) +")", SPARK_PUBLISH_TTL, PRIVATE);
                }
                else{
                    Particle.publish(SPARK_PUBLISH_TOPIC, "ts::readStringField(channelNumber: " + String(channelNumber) + " field: " + String(field) + ")", SPARK_PUBLISH_TTL, PRIVATE);
                }
            #endif
            return readRaw(channelNumber, String(String("/fields/") + String(field) + String("/last")), readAPIKey);
        }
        

        /*
        Function: readStringField
        
        Summary:
        Read the latest string from a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to read from.
        
        Returns:
        Value read (UTF8 string), or empty string if there is an error.  Use getLastReadStatus() to get more specific information.
        */
        String readStringField(unsigned long channelNumber, unsigned int field)
        {
            return readStringField(channelNumber, field, NULL);
        }


        /*
        Function: readFloatField
        
        Summary:
        ead the latest floating point value from a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to read from.
        readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Returns:
        Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.
        */
        float readFloatField(unsigned long channelNumber, unsigned int field, const char * readAPIKey)
        {
            return convertStringToFloat(readStringField(channelNumber, field, readAPIKey));
        }


        /*
        Function: readFloatField
        
        Summary:
        Read the latest floating point value from a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to read from.
        
        Returns:
        Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.
        */
        float readFloatField(unsigned long channelNumber, unsigned int field)
        {
            return readFloatField(channelNumber, field, NULL);
        }


        /*
        Function: readLongField
        
        Summary:
        Read the latest long value from a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to read from.
        readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Returns:
        Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.
        */
        long readLongField(unsigned long channelNumber, unsigned int field, const char * readAPIKey)
        {
            // Note that although the function is called "toInt" it really returns a long.
            return readStringField(channelNumber, field, readAPIKey).toInt();
        }


        /*
        Function: readLongField
        
        Summary:
        Read the latest long value from a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to read from.
        
        Returns:
        Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.
        */
        long readLongField(unsigned long channelNumber, unsigned int field)
        {
            return readLongField(channelNumber, field, NULL);
        }


        /*
        Function: readIntField
        
        Summary:
        Read the latest int value from a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to read from.
        readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Returns:
        Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.
        */
        int readIntField(unsigned long channelNumber, unsigned int field, const char * readAPIKey)
        {
            return readLongField(channelNumber, field, readAPIKey);
        }


        /*
        Function: readIntField
        
        Summary:
        Read the latest int value from a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        field - Field number (1-8) within the channel to read from.
        
        Returns:
        Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.
        */
        int readIntField(unsigned long channelNumber, unsigned int field)
        {
            return readLongField(channelNumber, field, NULL);
        }
        

        /*
        Function: readStatus
        
        Summary:
        Read the latest status from a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Results:
        Value read (UTF8 string). An empty string is returned if there was no status written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.
        */
        String readStatus(unsigned long channelNumber, const char * readAPIKey)
        {
            String content = readRaw(channelNumber, "/feeds/last.txt?status=true", readAPIKey);
            
            if(getLastReadStatus() != TS_OK_SUCCESS){
                return String("");
            }
            
            return getJSONValueByKey(content, "status");
        }
        
        
        /*
        Function: readStatus
        
        Summary:
        Read the latest status from a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        
        Results:
        Value read (UTF8 string). An empty string is returned if there was no status written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.
        */
        String readStatus(unsigned long channelNumber)
        {
            return readStatus(channelNumber, NULL);
        }
        
        
        /*
        Function: readCreatedAt
        
        Summary:
        Read the created-at timestamp associated with the latest update to a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Results:
        Value read (UTF8 string). An empty string is returned if there was no created-at timestamp written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.
        */
        String readCreatedAt(unsigned long channelNumber, const char * readAPIKey)
        {
            String content = readRaw(channelNumber, "/feeds/last.txt", readAPIKey);
            
            if(getLastReadStatus() != TS_OK_SUCCESS){
                return String("");
            }
            
            return getJSONValueByKey(content, "created_at");
        }
        

        /*
        Function: readCreatedAt
        
        Summary:
        Read the created-at timestamp associated with the latest update to a private ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
            
        Results:
        Value read (UTF8 string). An empty string is returned if there was no created-at timestamp written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.
        */
        String readCreatedAt(unsigned long channelNumber)
        {
            return readCreatedAt(channelNumber, NULL);
        }
        
        
        /*
        Function: readRaw
        
        Summary:
        Read a raw response from a public ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        URLSuffix - Raw URL to write to ThingSpeak as a String.  See the documentation at https://thingspeak.com/docs/channels#get_feed
        readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*
        
        Returns:
        Response if successful, or empty string. Use getLastReadStatus() to get more specific information.
        
        Notes:
        This is low level functionality that will not be required by most users.
        */
        String readRaw(unsigned long channelNumber, String URLSuffix, const char * readAPIKey)
        {
            #ifdef PRINT_DEBUG_MESSAGES
                if(NULL != readAPIKey)
                {
                    Particle.publish(SPARK_PUBLISH_TOPIC, "ts::readRaw   (channelNumber: " + String(channelNumber) + " readAPIKey: " + String(readAPIKey) + " URLSuffix: \"" + URLSuffix + "\")" , SPARK_PUBLISH_TTL, PRIVATE);
                }
                else
                {
                    Particle.publish(SPARK_PUBLISH_TOPIC, "ts::readRaw   (channelNumber: " + String(channelNumber) + " URLSuffix: \"" + URLSuffix + "\")" , SPARK_PUBLISH_TTL, PRIVATE);
                }
            #endif

            if(!connectThingSpeak())
            {
                this->lastReadStatus = TS_ERR_CONNECT_FAILED;
                return String("");
            }

            String URL = String("/channels/") + String(channelNumber) + URLSuffix;

            #ifdef PRINT_DEBUG_MESSAGES
            Particle.publish(SPARK_PUBLISH_TOPIC,"               GET \"" + URL + "\"" , SPARK_PUBLISH_TTL, PRIVATE);
            #endif

            // Post data to thingspeak
            if(!this->client->print("GET ")) return abortReadRaw();
            if(!this->client->print(URL)) return abortReadRaw();
            if(!this->client->print(" HTTP/1.1\r\n")) return abortReadRaw();
            if(!writeHTTPHeader(readAPIKey)) return abortReadRaw();
            if(!this->client->print("\r\n")) return abortReadRaw();
     
            String content = String();
            int status = getHTTPResponse(content);
            this->lastReadStatus = status;


            #ifdef PRINT_DEBUG_MESSAGES
                if(status == TS_OK_SUCCESS)
                {
                    Particle.publish(SPARK_PUBLISH_TOPIC, "Read: \"" + content + "\"" , SPARK_PUBLISH_TTL, PRIVATE);
                }
            #endif

            client->stop();
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "disconnected." , SPARK_PUBLISH_TTL, PRIVATE);
            #endif

            if(status != TS_OK_SUCCESS)
            {
                // return status;
                return String("");
            }

            // This is a workaround to a bug in the Spark implementation of String
            return String("") + content;
        }
        
        
        /*
        Function: readRaw
        
        Summary:
        Read a raw response from a public ThingSpeak channel
        
        Parameters:
        channelNumber - Channel number
        URLSuffix - Raw URL to write to ThingSpeak as a String.  See the documentation at https://thingspeak.com/docs/channels#get_feed
        
        Returns:
        Response if successful, or empty string. Use getLastReadStatus() to get more specific information.
        
        Notes:
        This is low level functionality that will not be required by most users.
        */
        String readRaw(unsigned long channelNumber, String URLSuffix)
        {
            return readRaw(channelNumber, URLSuffix, NULL);
        }
        
        
        /*
        Function: readMultipleFields
         
        Summary:
        Read all the field values, status message, location coordinates, and created-at timestamp associated with the latest feed to a private ThingSpeak channel and store the values locally in variables within a struct.
         
        Parameters:
        channelNumber - Channel number
        readAPIKey - Read API key associated with the channel. *If you share code with others, do _not_ share this key*
         
        Returns:
        HTTP status code of 200 if successful.
        
        Notes:
        See getLastReadStatus() for other possible return values.
        */
        int readMultipleFields(unsigned long channelNumber, const char * readAPIKey)
        {
            String readCondition = "/feeds/last.txt?status=true&location=true";
            
            String multiContent = readRaw(channelNumber, readCondition, readAPIKey);
            
            if(getLastReadStatus() != TS_OK_SUCCESS){
                return getLastReadStatus();
            }
            
            this->lastFeed.nextReadField[0] = parseValues(multiContent, "field1");
            this->lastFeed.nextReadField[1] = parseValues(multiContent, "field2");
            this->lastFeed.nextReadField[2] = parseValues(multiContent, "field3");
            this->lastFeed.nextReadField[3] = parseValues(multiContent, "field4");
            this->lastFeed.nextReadField[4] = parseValues(multiContent, "field5");
            this->lastFeed.nextReadField[5] = parseValues(multiContent, "field6");
            this->lastFeed.nextReadField[6] = parseValues(multiContent, "field7");
            this->lastFeed.nextReadField[7] = parseValues(multiContent, "field8");
            this->lastFeed.nextReadCreatedAt = parseValues(multiContent, "created_at");
            this->lastFeed.nextReadLatitude = parseValues(multiContent, "latitude");
            this->lastFeed.nextReadLongitude = parseValues(multiContent, "longitude");
            this->lastFeed.nextReadElevation = parseValues(multiContent, "elevation");
            this->lastFeed.nextReadStatus = parseValues(multiContent, "status");
            
            return TS_OK_SUCCESS;
        }
        
        
        /*
        Function: readMultipleFields
        
        Summary:
        Read all the field values, status message, location coordinates, and created-at timestamp associated with the latest update to a private ThingSpeak channel and store the values locally in variables within a struct.
         
        Parameters:
        channelNumber - Channel number
        readAPIKey - Read API key associated with the channel. *If you share code with others, do _not_ share this key*
         
        Returns:
        HTTP status code of 200 if successful.
        
        Notes:
        See getLastReadStatus() for other possible return values.
        */
        int readMultipleFields(unsigned long channelNumber)
        {
            return readMultipleFields(channelNumber, NULL);
        }
        
        
        /*
        Function: getFieldAsString
         
        Summary:
        Fetch the value as string from the latest stored feed record.
        
        Parameters:
        field - Field number (1-8) within the channel to read from.
        
        Returns:
        Value read (UTF8 string), empty string if there is an error, or old value read (UTF8 string) if invoked before readMultipleFields().  Use getLastReadStatus() to get more specific information.
        */
        String getFieldAsString(unsigned int field)
        {
            if(field < FIELDNUM_MIN || field > FIELDNUM_MAX)
            {
                this->lastReadStatus = TS_ERR_INVALID_FIELD_NUM;
                return("");
            }
            
            this->lastReadStatus = TS_OK_SUCCESS;
            return this->lastFeed.nextReadField[field-1];
        }
        
        
        /*
        Function: getFieldAsFloat
         
        Summary:
        Fetch the value as float from the latest stored feed record.
        
        Parameters:
        field - Field number (1-8) within the channel to read from.
        
        Returns:
        Value read, 0 if the field is text or there is an error, or old value read if invoked before readMultipleFields(). Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.
        */
        float getFieldAsFloat(unsigned int field)
        {
            return convertStringToFloat(getFieldAsString(field));
        }
        
        
        /*
        Function: getFieldAsLong
        
        Summary:
        Fetch the value as long from the latest stored feed record.
        
        Parameters:
        field - Field number (1-8) within the channel to read from.
        
        Returns:
        Value read, 0 if the field is text or there is an error, or old value read if invoked before readMultipleFields(). Use getLastReadStatus() to get more specific information.
        */
        long getFieldAsLong(unsigned int field)
        {
            // Note that although the function is called "toInt" it really returns a long.
            return getFieldAsString(field).toInt();
        }
        
        
        /*
        Function: getFieldAsInt
         
        Summary:
        Fetch the value as int from the latest stored feed record.
        
        Parameters:
        field - Field number (1-8) within the channel to read from.
        
        Returns:
        Value read, 0 if the field is text or there is an error, or old value read if invoked before readMultipleFields(). Use getLastReadStatus() to get more specific information.
        */
        int getFieldAsInt(unsigned int field)
        {
            // int and long are same
            return getFieldAsLong(field);
        }
        
        
        /*
        Function: getStatus
         
        Summary:
        Fetch the status message associated with the latest stored feed record.
        
        Results:
        Value read (UTF8 string). An empty string is returned if there was no status written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.
        */
        String getStatus()
        {
            return this->lastFeed.nextReadStatus;
        }
        
        
        /*
        Function: getLatitude
         
        Summary:
        Fetch the latitude associated with the latest stored feed record.
        
        Results:
        Value read (UTF8 string). An empty string is returned if there was no latitude written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.
        */
        String getLatitude()
        {
            return this->lastFeed.nextReadLatitude;
        }
        
        
        /*
        Function: getLongitude
         
        Summary:
        Fetch the longitude associated with the latest stored feed record.
        
        Results:
        Value read (UTF8 string). An empty string is returned if there was no longitude written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.
        */
        String getLongitude()
        {
            return this->lastFeed.nextReadLongitude;
        }
        
        
        /*
        Function: getElevation
         
        Summary:
        Fetch the longitude associated with the latest stored feed record.
        
        Results:
        Value read (UTF8 string). An empty string is returned if there was no elevation written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.
        */
        String getElevation()
        {
            return this->lastFeed.nextReadElevation;
        }
        
        
        /*
        Function: getCreatedAt
         
        Summary:
        Fetch the created-at timestamp associated with the latest stored feed record.
        
        Results:
        Value read (UTF8 string). An empty string is returned if there was no created-at timestamp written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.
        */
        String getCreatedAt()
        {
            return this->lastFeed.nextReadCreatedAt;
        }
        
        
        /*
        Function: getLastReadStatus
        
        Summary:
        Get the status of the previous read.
        
        Returns:
        Generally, these are HTTP status codes.  Negative values indicate an error generated by the library.
        Possible response codes...
        200 - OK / Success
        404 - Incorrect API key (or invalid ThingSpeak server address)
        -101 - Value is out of range or string is too long (> 255 characters)
        -201 - Invalid field number specified
        -210 - setField() was not called before writeFields()
        -301 - Failed to connect to ThingSpeak
        -302 -  Unexpected failure during write to ThingSpeak
        -303 - Unable to parse response
        -304 - Timeout waiting for server to respond
        -401 - Point was not inserted (most probable cause is exceeding the rate limit)
        
        Notes:
        The read functions will return zero or empty if there is an error.  Use this function to retrieve the details.
        */
        int getLastReadStatus()
        {
            return this->lastReadStatus;
        }
        
        
    private:
        
        // Creates a new String
        String escapeUrl(String message){
            char t;
            char ch[] = " ";
            char temp[4];
            char *encoded;
            String result = "";
            unsigned int i;
            unsigned int n = message.length() + 1;  // add an extra for the null
            
            // figure out the length of the char array
            for(i = 0; i < message.length(); i++){
                t = message.charAt(i);
                if( (t >= 0x00 && t <= 0x1F) || t >= 0x80 ){
                    n--;
                    continue;
                }
                if(t == 0x22 || t == 0x25 || t == 0x26 || t == 0x2B || t == 0x3B){
                    n = n + 2;
                }
            }
            
            // create the char array
            encoded = (char *)malloc(sizeof(char) * n);
            if(encoded == NULL){
                return result;
            }
            encoded[0] = 0;
                
            // build the char array
            for(i = 0; i < message.length(); i++){
                t = message.charAt(i);
                // don't include non-printable or anything about 127
                if( (t >= 0x00 && t <= 0x1F) || t >= 0x80 ){
                    continue;
                }
                // encode the special characters
                if(t == 0x22 || t == 0x25 || t == 0x26 || t == 0x2B || t == 0x3B){
                    sprintf(temp, "%%%02X", t);
                    strcat(encoded, temp);
                    continue;
                }
                // add the regular characters
                ch[0] = t;
                strcat(encoded, ch);
            }
                
            result = String(encoded);
            free(encoded);
            
            return result;
        }
        
        String getJSONValueByKey(String textToSearch, String key)
        {
            if(textToSearch.length() == 0){
                return String("");
            }
            
            String searchPhrase = String("\"") + key + String("\":\"");
            
            int fromPosition = textToSearch.indexOf(searchPhrase,0);
            
            if(fromPosition == -1){
                // return because there is no status or it's null
                return String("");
            }
            
            fromPosition = fromPosition + searchPhrase.length();
                    
            int toPosition = textToSearch.indexOf("\"", fromPosition);
            
            
            if(toPosition == -1){
                // return because there is no end quote
                return String("");
            }
            
            textToSearch.remove(toPosition);
            
            return textToSearch.substring(fromPosition);
        }
        
        String parseValues(String & multiContent, String key)
        {
            if(multiContent.length() == 0){
                return String("");
            }
            
            String searchPhrase = String("\"") + key + String("\":\"");
            
            int fromPosition = multiContent.indexOf(searchPhrase,0);
            
            if(fromPosition == -1){
                // return because there is no status or it's null
                return String("");
            }
            
            fromPosition = fromPosition + searchPhrase.length();
                    
            int toPosition = multiContent.indexOf("\"", fromPosition);
            
            
            if(toPosition == -1){
                // return because there is no end quote
                return String("");
            }
            
            return multiContent.substring(fromPosition, toPosition);
        }
        
        int abortWriteRaw()
        {
            this->client->stop();
            return TS_ERR_UNEXPECTED_FAIL;
        }

        String abortReadRaw()
        {
            this->client->stop();
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, "ReadRaw abort - disconnected." , SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            this->lastReadStatus = TS_ERR_UNEXPECTED_FAIL;
            return String("");
        }
        
        void setPort(unsigned int port)
        {
            this->port = port;
        }
        
        
        void setClient(Client * client)
        {
            this->client = client;
        }

        Client * client = NULL;
        unsigned int port = THINGSPEAK_PORT_NUMBER;
        String nextWriteField[8];
        float nextWriteLatitude;
        float nextWriteLongitude;
        float nextWriteElevation;
        int lastReadStatus;
        String nextWriteStatus;
        String nextWriteCreatedAt;
        feed lastFeed;

        bool connectThingSpeak()
        {
            bool connectSuccess = false;
            
            #ifdef PRINT_DEBUG_MESSAGES
                Particle.publish(SPARK_PUBLISH_TOPIC, String("Connect to ThingSpeak URL: ") + String(THINGSPEAK_URL) + String(":") + String(this->port) + "..." , SPARK_PUBLISH_TTL, PRIVATE);
                Serial.print(THINGSPEAK_URL);
                Serial.print(":");
                Serial.print(this->port);
                Serial.print("...");
            #endif
            connectSuccess = client->connect(THINGSPEAK_URL, this->port);
            
            #ifdef PRINT_DEBUG_MESSAGES
            if (connectSuccess)
            {
                Particle.publish(SPARK_PUBLISH_TOPIC, "Connection Success", SPARK_PUBLISH_TTL, PRIVATE);
            }
            else
            {
                Particle.publish(SPARK_PUBLISH_TOPIC, "Connection Failure", SPARK_PUBLISH_TTL, PRIVATE);
            }
            #endif
            return connectSuccess;
            
        };

        bool writeHTTPHeader(const char * APIKey)
        {
            
            if (!this->client->print("Host: api.thingspeak.com\r\n")) return false;
            if (!this->client->print("Connection: close\r\n")) return false;
            if (!this->client->print("User-Agent: ")) return false;
            if (!this->client->print(TS_USER_AGENT)) return false;
            if (!this->client->print("\r\n")) return false;
            if(NULL != APIKey)
            {
                if (!this->client->print("X-THINGSPEAKAPIKEY: ")) return false;
                if (!this->client->print(APIKey)) return false;
                if (!this->client->print("\r\n")) return false;
            }
            return true;
        };

        int getHTTPResponse(String & response)
        {
            unsigned long startWaitForResponseAt = millis();
            while(client->available() == 0 && millis() - startWaitForResponseAt < TIMEOUT_MS_SERVERRESPONSE)
            {
                delay(100);
            }
            if(client->available() == 0)
            {
                return TS_ERR_TIMEOUT; // Didn't get server response in time
            }

            if(!client->find(const_cast<char *>("HTTP/1.1")))
            {
                #ifdef PRINT_HTTP
                    Particle.publish(SPARK_PUBLISH_TOPIC, "ERROR: Didn't find HTTP/1.1", SPARK_PUBLISH_TTL, PRIVATE);
                #endif
                return TS_ERR_BAD_RESPONSE; // Couldn't parse response (didn't find HTTP/1.1)
            }
            int status = client->parseInt();
            #ifdef PRINT_HTTP
                Particle.publish(SPARK_PUBLISH_TOPIC, "Got Status of " + String(status), SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            if(status != TS_OK_SUCCESS)
            {
                return status;
            }

            if(!client->find(const_cast<char *>("\r\n")))
            {
                #ifdef PRINT_HTTP
                    Particle.publish(SPARK_PUBLISH_TOPIC, "ERROR: Didn't find end of status line", SPARK_PUBLISH_TTL, PRIVATE);
                #endif
                return TS_ERR_BAD_RESPONSE;
            }
            #ifdef PRINT_HTTP
                Particle.publish(SPARK_PUBLISH_TOPIC, "Found end of status line", SPARK_PUBLISH_TTL, PRIVATE);
            #endif

            if(!client->find(const_cast<char *>("\n\r\n")))
            {
                #ifdef PRINT_HTTP
                    Particle.publish(SPARK_PUBLISH_TOPIC, "ERROR: Didn't find end of header", SPARK_PUBLISH_TTL, PRIVATE);
                #endif
                return TS_ERR_BAD_RESPONSE;
            }
            #ifdef PRINT_HTTP
                Particle.publish(SPARK_PUBLISH_TOPIC, "Found end of header", SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            // This is a workaround to a bug in the Spark implementation of String
            String tempString = client->readString();
            response = tempString;
            #ifdef PRINT_HTTP
                Particle.publish(SPARK_PUBLISH_TOPIC, "Response: \"" + tempString + "\"", SPARK_PUBLISH_TTL, PRIVATE);
            #endif
            return status;
        };

        int convertFloatToChar(float value, char *valueString)
        {
            // Supported range is -999999000000 to 999999000000
            if(0 == isinf(value) && (value > 999999000000 || value < -999999000000))
            {
                // Out of range
                return TS_ERR_OUT_OF_RANGE;
            }
            // Given that the resolution of Spark is 1 / 2^12, or ~0.00024 volts, assume that 5 places right of decimal should be sufficient for most applications
            sprintf(valueString, "%.5f", value);

            return TS_OK_SUCCESS;
        };

        float convertStringToFloat(String value)
        {
            // There's a bug in the AVR function strtod that it doesn't decode -INF correctly (it maps it to INF)
            float result = value.toFloat();
            if(1 == isinf(result) && *value.c_str() == '-')
            {
                result = (float)-INFINITY;
            }
            return result;
        };

        void resetWriteFields()
        {
            for(size_t iField = 0; iField < 8; iField++)
            {
                this->nextWriteField[iField] = "";
            }
            this->nextWriteLatitude = NAN;
            this->nextWriteLongitude = NAN;
            this->nextWriteElevation = NAN;
            this->nextWriteStatus = "";
            this->nextWriteCreatedAt = "";
        };
    };

    extern ThingSpeakClass ThingSpeak;

#endif //ThingSpeak_h
