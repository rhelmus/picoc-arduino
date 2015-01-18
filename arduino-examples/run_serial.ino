#include <Arduino.h>
#include <picoc.h>
//#include <SdFat.h> // UNDONE

namespace {

enum
{
    BUFFER_LEN = 256 // NOTE: picoc has an internal buffer length as well (LINEBUFFER_MAX)
};

Picoc pc;

}

void setup()
{
    while (!Serial)
        ;

    delay(3000);

    Serial.begin(115200);

    Serial.println("Everything initialized.");

}

void loop()
{
#if 0
    static uint32_t delay;
    const uint32_t curtime = millis();

    if (curtime >= delay)
    {
        delay = curtime + 50;

        if (Serial.available())
        {
            char buffer[BUFFER_LEN];
            if (Serial.readBytesUntil('\n', buffer, BUFFER_LEN-1) > 0)
            {
                buffer[BUFFER_LEN-1] = 0;

            }
        }
    }
#endif
    PicocInitialise(&pc, HEAP_SIZE);
    Serial.println("Starting serial interactive picoc. Hit ctrl+d (EOF) to reset.");
    PicocParseInteractive(&pc); // blocks
    PicocCleanup(&pc);
}
