#include <RDM6300Helper.h>

const uint16_t NUM_READERS = 3;
const uint16_t NUM_TAG_OPTIONS = 2;
const unsigned long POLL_MS = 1500;

RDM6300 rfid01(2, 3);
RDM6300 rfid02(4, 5);
RDM6300 rfid03(6, 7);

RDM6300 rfids[NUM_READERS] = {
    rfid01,
    rfid02,
    rfid03
};

String tags[NUM_READERS][NUM_TAG_OPTIONS] = {
    { "100079521800", "100079521801" },
    { "100079634300", "100079634301" },
    { "100079810300", "100079810301" }
};

RDM6300Helper rfidHelper = RDM6300Helper(
    rfids,
    NUM_READERS,
    tags[0],
    NUM_TAG_OPTIONS,
    POLL_MS);

void onTagCallback(int idxReader, String tag)
{
    Serial.print(F("Tag: "));
    Serial.println(tag);
}

void setup()
{
    rfidHelper.initRfids();
    rfidHelper.addCallback(onTagCallback);
}

void loop()
{
    rfidHelper.pollRfids();
}