#ifndef _RFID_HELPER_
#define _RFID_HELPER_

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <rdm630.h>

const int MAX_RD6300_CALLBACKS = 5;

typedef void (*rdm6300_callback_func)(int, String);

typedef struct
{
    rdm6300_callback_func callback;
    uint16_t minReads;
    int idxReader;
    bool onlyOnce;
    bool anyTag;
} RDM6300Callback;

class RDM6300Helper {
public:
    RDM6300Helper(
        RDM6300* rfids,
        size_t sizeRfids,
        String** validTags,
        size_t sizeValidTags,
        unsigned long pollMs = 1000);

    ~RDM6300Helper();

    void initRfids();
    void pollRfids();
    void addCallback(rdm6300_callback_func, uint16_t, int, bool, bool);

private:
    RDM6300* rfids;
    size_t sizeRfids;
    String** validTags;
    size_t sizeValidTags;
    unsigned long pollMs;
    uint16_t* consecutiveReads;
    String* currentTags;
    unsigned long lastPoll = 0;
    int cbCounter = 0;
    RDM6300Callback callbacks[MAX_RD6300_CALLBACKS];

    bool isValidTag(int);
    bool shouldPoll();
    void readRfid(int);
    void runCallbacks();
    void runCallback(int, int);
};

#endif