# TFmini plus

Arduino library for [Benewake TFmini plus micro LiDAR module](https://en.benewake.com/TFminiPlus/). 
Forked from hideakitai's [TFmini library](https://github.com/hideakitai/TFmini).

## Usage

```
#include "TFminiPlus.h"

TFminiPlus tfmini;

void setup()
{
    Serial.begin(115200);
    Serial1.begin(TFminiPlus::DEFAULT_BAUDRATE);
    tfmini.attach(Serial1);
}

void loop()
{
    if (tfmini.available())
    {
        Serial.print("distance : ");
        Serial.println(tfmini.getDistance());
        Serial.print("strength : ");
        Serial.println(tfmini.getStrength());
        Serial.print("temperature : ");
        Serial.println(tfmini.getTemperature());
    }
}
```

### Supported Configuration

```
// default : true
void enable(bool state);

// default : Standard_ms
void setOutputDataFormat(const OutputDataFormat format);

// default : 100fps
void setOutputDataPeriod(const FrameRate fps);
void setOutputDataPeriod(uint16_t fps);

// manual trigger for the measurement
void trigger();

// default : 115200 (0x06)
void setBaudRate(Baudrate baud);

// reset all settings
void resetSettings();

// save settings
void saveSettings();

// system reset
void systemReset();
```

### Public Constants

```
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
```

## Note

Pixhawk data format is not supported yet

## License

MIT
