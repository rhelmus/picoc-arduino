#include <Arduino.h>
#include <picoc.h>
#include "serialram.h"
#include "virtmem.h"
#include <Wire.h>

//#include <SdFat.h> // UNDONE
#include <SPI.h>

namespace {

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
    PicocInitialise(&pc, HEAP_SIZE);
    Serial.println("Starting serial interactive picoc. Run exit() to reset.");
    PicocEnablePrompt(&pc, false);
    PicocParseInteractive(&pc); // blocks
//    Serial.print("Exiting... "); Serial.print(MaxStackMemUsed(&pc)); Serial.print("/"); Serial.print(MaxHeapMemUsed(&pc)); Serial.println(" bytes used");
    PicocCleanup(&pc);
}
