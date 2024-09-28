#pragma once

#ifndef TFMINIPLUS_H
#define TFMINIPLUS_H

class TFminiPlus
{
public:
    static const uint32_t DEFAULT_BAUDRATE = 115200;

    enum class OutputDataFormat
    {
        Standard_cm = 0x01,
        Pixhawk = 0x02,
        Standard_mm = 0x06
    };
    enum class Baudrate
    {
        BAUD_9600 = 0x002580,
        BAUD_14400 = 0x003840,
        BAUD_19200 = 0x004B00,
        BAUD_38400 = 0x009600,
        BAUD_56000 = 0x00DAC0,
        BAUD_57600 = 0x00E100,
        BAUD_115200 = 0x01C200,
        BAUD_128000 = 0x01F400,
        BAUD_230400 = 0x038400,
        BAUD_256000 = 0x03E800,
        BAUD_460800 = 0x070800,
        BAUD_500000 = 0x07A120,
        BAUD_512000 = 0x07D000
    };
    enum class FrameRate
    {
        FPS_0 = 0x0000, 
        FPS_1 = 0x0001,
        FPS_2 = 0x0002,
        FPS_5 = 0x0005,
        FPS_10 = 0x000A,
        FPS_20 = 0x0014,
        FPS_25 = 0x0019,
        FPS_50 = 0x0032,
        FPS_100 = 0x0064,
        FPS_125 = 0x007D,
        FPS_200 = 0x00C8,
        FPS_250 = 0x00FA,
        FPS_500 = 0x01F4,
        FPS_1000 = 0x03E8
    };

    void attach(Stream &s) { stream_ = &s; }

    bool available()
    {
        if (!stream_)
            return false;

        update();
        if (b_available_)
        {
            b_available_ = false;
            return true;
        }
        else
            return false;
    }

    uint16_t getDistance() const { return packet_.distance.i; }
    uint16_t getStrength() const { return packet_.strength.i; }
    uint16_t getTemperature() const { return packet_.temperature.i; }


    void enable(bool state)
    {
        stream_->write(COMMAND_HEADER);
        stream_->write((uint8_t)0x05);
        stream_->write((uint8_t)0x07);
        if (state)
        {
            stream_->write((uint8_t)0x01);
            stream_->write((uint8_t)0x67);
        }
        else
        {
            stream_->write((uint8_t)0x00);
            stream_->write((uint8_t)0x66);
        }
    }
    void trigger()
    {
        stream_->write(COMMAND_HEADER);
        stream_->write((uint8_t)0x04);
        stream_->write((uint8_t)0x04);
        stream_->write((uint8_t)0x62);   
    }
    // default : Standard_cm
    void setOutputDataFormat(const OutputDataFormat fmt)
    {
        format_ = fmt;
        stream_->write(COMMAND_HEADER);
        stream_->write((uint8_t)0x05);
        stream_->write((uint8_t)0x05);
        stream_->write((uint8_t)fmt);
        stream_->write((uint8_t)(0x64+(uint8_t)fmt));
    }

    // default : 10ms
    void setOutputDataPeriod(const FrameRate fps)
    {
        uint8_t sum = 99; // 0x5A+0x06+0x03
        stream_->write(COMMAND_HEADER);
        stream_->write((uint8_t)0x06);
        stream_->write((uint8_t)0x03);
        uint8_t temp = ((uint16_t)fps >> 0) & 0x00FF;
        stream_->write(temp);
        sum +=temp;
        temp = ((uint16_t)fps >> 8) & 0x00FF;
        stream_->write(temp);
        sum += temp;
        stream_->write(temp);
    }
    void setOutputDataPeriod(uint16_t fps)
    {
        uint8_t sum = 99; // 0x5A+0x06+0x03
        stream_->write(COMMAND_HEADER);
        stream_->write((uint8_t)0x06);
        stream_->write((uint8_t)0x03);
        uint8_t temp = (fps >> 8) & 0x00FF;
        stream_->write(temp);
        sum +=temp;
        temp = (fps >> 0) & 0x00FF;
        stream_->write(temp);
        sum += temp;
        stream_->write(temp);
    }

    // default : 115200 (0x01C200)
    void setBaudRate(Baudrate baud)
    {
        stream_->write(COMMAND_HEADER);
        stream_->write((uint8_t)0x08);
        stream_->write((uint8_t)0x06);
        uint8_t temp = 0, sum = 104;
        for (uint8_t i = 0; i < 4; i++)
        {
            temp = ((uint32_t)baud >> (i*8)) & 0xFF;
            stream_->write(temp);
            sum += temp;
        }
        stream_->write((uint8_t)sum);
    }

    // reset all settings
    void resetSettings()
    {
        stream_->write(COMMAND_HEADER);
        stream_->write((uint8_t)0x04);
        stream_->write((uint8_t)0x10);
        stream_->write((uint8_t)0x6E);
    }

    //Save settings
    void saveSettings()
    {
        stream_->write(COMMAND_HEADER);
        stream_->write((uint8_t)0x04);
        stream_->write((uint8_t)0x11);
        stream_->write((uint8_t)0x6F);
    }

    //System reset
    void systemReset()
    {
        stream_->write(COMMAND_HEADER);
        stream_->write((uint8_t)0x04);
        stream_->write((uint8_t)0x02);
        stream_->write((uint8_t)0x60);
    }

private:
    void update()
    {
        while (stream_->available())
        {
            uint8_t data = (uint8_t)stream_->read();

            if (format_ == OutputDataFormat::Pixhawk)
            {
                Serial.println("Pixhawk Format NOT SUPPORTED YET");
                return;
            }

            if (state_ != State::CHECKSUM)
                buffer_.sum += data;

            switch (state_)
            {
            case State::HEAD_L:
            {
                reset();
                buffer_.sum = data;
                if (data == RECV_FRAME_HEADER)
                    state_ = State::HEAD_H;
                break;
            }
            case State::HEAD_H:
            {
                if (data == RECV_FRAME_HEADER)
                    state_ = State::DIST_L;
                else
                    state_ = State::HEAD_L;
                break;
            }
            case State::DIST_L:
            {
                buffer_.distance.b[0] = data;
                state_ = State::DIST_H;
                break;
            }
            case State::DIST_H:
            {
                buffer_.distance.b[1] = data;
                state_ = State::STRENGTH_L;
                break;
            }
            case State::STRENGTH_L:
            {
                buffer_.strength.b[0] = data;
                state_ = State::STRENGTH_H;
                break;
            }
            case State::STRENGTH_H:
            {
                buffer_.strength.b[1] = data;
                state_ = State::TEMP_L;
                break;
            }
            case State::TEMP_L:
            {
                buffer_.temperature.b[0] = data;
                state_ = State::TEMP_H;
                break;
            }
            case State::TEMP_H:
            {
                buffer_.temperature.b[1] = data;
                state_ = State::CHECKSUM;
                break;
            }
            case State::CHECKSUM:
            {
                if (buffer_.sum == data)
                {
                    packet_ = buffer_;
                    b_available_ = true;
                }
                else
                {
                    b_available_ = false;
                }
                reset();
                break;
            }
            default:
            {
                reset();
                break;
            }
            }
        }
    }

    void reset()
    {
        buffer_.clear();
        state_ = State::HEAD_L;
    }

    struct Packet
    {
        union
        {
            uint8_t b[2];
            uint16_t i;
        } distance;
        union
        {
            uint8_t b[2];
            uint16_t i;
        } strength;
        union
        {
            uint8_t b[2];
            uint16_t i;
        } temperature;
        uint8_t sum;

        void clear() { distance.i = strength.i = temperature.i = sum = 0; }
    };

    enum class State
    {
        HEAD_L,
        HEAD_H,
        DIST_L,
        DIST_H,
        STRENGTH_L,
        STRENGTH_H,
        TEMP_L,
        TEMP_H,
        CHECKSUM
    };

    static const uint8_t 
        RECV_FRAME_HEADER = 0x59,
        COMMAND_HEADER = 0x5A;

    Packet packet_;
    Packet buffer_;
    State state_;

    bool b_available_;
    Stream *stream_;

    OutputDataFormat format_{OutputDataFormat::Standard_cm};
};

#endif // TFMINI_H