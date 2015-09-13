#include <EEPROM.h>

enum proxymode_t { PASSTHROUGH, RECORD, PLAYBACK };

class ControlProxy {
  private:
    proxymode_t mode;
    int since;
    byte prevc;
    int addr;

  public:
    void begin(proxymode_t m = PASSTHROUGH) {
      Control.begin();
      mode = m;
      since = 0;
      prevc = Control.read();
      addr = 0;
      if (m == RECORD) {
        EEPROM.write(addr++, prevc);
        since = -1;
      }
    }
    byte read(void) {
      byte c;
      switch (mode) {
      case PASSTHROUGH:
        return Control.read();
      case RECORD:
        c = Control.read();
        if (c == prevc) {
          since++;
        } else {
          // means: wait N then deliver c
          EEPROM.write(addr++, lowByte(since));
          EEPROM.write(addr++, highByte(since));
          EEPROM.write(addr++, c);
          since = 0;
          prevc = c;
        }
        return c;
      case PLAYBACK:
        if (since) {
          since--;
        } else {
          prevc = EEPROM.read(addr++);
          since = EEPROM.read(addr) | (EEPROM.read(addr + 1) << 8);
          addr += 2;
        }
        return prevc;
      }
    }
};

static ControlProxy CP;
#define Control CP

#if 0   // demo
  for (byte p = 0; ; p ^= 1) {
    Serial.print("GO "); Serial.println(p, DEC);
    Control.begin((p == 0) ? RECORD : PLAYBACK);
    for (int x = 0; x < 300; x++) {
      GD.waitvblank();
      if (Control.read())
        Serial.println(x, DEC);
    }
  }
#endif
