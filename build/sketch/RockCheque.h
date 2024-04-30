#line 1 "C:\\Users\\Adam\\Documents\\dev\\rock-cheque-v2-control\\RockCheque.h"
#include "Arduino.h"

class I2C {
public:
    I2C();
    void write(uint8_t addr, uint8_t reg, uint8_t byte);
    uint8_t read(uint8_t addr, uint8_t reg);
};

class DriverIC {
protected:
    uint8_t addr;
    I2C& i2c;
public:
    DriverIC(uint8_t addr, I2C& i2c);
    virtual void setPWM(uint8_t pin, uint8_t duty);
};

class TLC59108 : public DriverIC {
private:
    const uint8_t MODE1 = 0x00;
    const uint8_t MODE2 = 0x01;
    const uint8_t PWM0 = 0x02;
    const uint8_t GRPPWM = 0x0A;
    const uint8_t GRPFREQ = 0x0B;
    const uint8_t LEDOUT0 = 0x0C;
    const uint8_t LEDOUT1 = 0x0D;
    const uint8_t SUBADR1 = 0x0E;
    const uint8_t SUBADR2 = 0x0F;
    const uint8_t SUBADR3 = 0x10;
    const uint8_t ALLCALLADR = 0x11;
    const uint8_t IREF = 0x12;
    const uint8_t EFLAG = 0x13;
    uint8_t duty;
public:
    TLC59108(uint8_t addr, I2C& i2c);
    void setPWM(uint8_t pin, uint8_t duty) override;

};

class PCA9685 : public DriverIC {
    
    const uint8_t MODE1 = 0x00;
    const uint8_t MODE2 = 0x01;
    const uint8_t SUBADR1 = 0x02;
    const uint8_t SUBADR2 = 0x03;
    const uint8_t SUBADR3 = 0x04;
    const uint8_t ALLCALLADR = 0x05;
    const uint8_t LED0_ON_L = 0x06;
    const uint8_t LED0_ON_H = 0x07;
    const uint8_t LED0_OFF_L = 0x08;
    const uint8_t LED0_OFF_H = 0x09;
    const uint8_t ALL_LED_ON_L = 0xFA;
    const uint8_t ALL_LED_ON_H = 0xFB;
    const uint8_t ALL_LED_OFF_L = 0xFC;
    const uint8_t ALL_LED_OFF_H = 0xFD;
    const uint8_t PRE_SCALE = 0xFE;

public:
    PCA9685(uint8_t addr, I2C& i2c);
    void setPWM(uint8_t pin, uint8_t duty) override;
};

class Digit {
    static const int SYMBOL_TO_SEGMENT_LOOKUP[36][7];
    DriverIC* driverIC;
    int* SegmentToPinLookup;
    uint8_t dutyCycle;
    uint8_t dotPointDutyCycle;

public:
    Digit(DriverIC* ic, int* segmentToPinLookup, uint8_t _dutyCycle, uint8_t _dotPointDutyCycle);
    void setSymbol(int symbol);
    void dotPoint(bool state);
    void clear();
};

class Display {
    Digit** digits;
    int count;
public:
    Display(Digit** _digits, int _count);
    bool setDigit(int digit, int symbol);
    void setNumber(int num);
    void setAll(int symbol);
    void clearDigit(int digit);
};

