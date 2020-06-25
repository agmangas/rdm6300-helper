#include "RDM6300Helper.h"

RDM6300Helper::RDM6300Helper(
    RDM6300* rfids,
    uint16_t sizeRfids,
    String* tags,
    uint16_t sizeTags,
    unsigned long pollMs)
    : rfids(rfids)
    , sizeRfids(sizeRfids)
    , tags(tags)
    , sizeTags(sizeTags)
    , pollMs(pollMs)
{
    consecutiveReads = new uint16_t[sizeRfids];
    currentTags = new String[sizeRfids];

    for (uint16_t i = 0; i < sizeRfids; i++) {
        consecutiveReads[i] = 0;
        currentTags[i] = "";
    }
}

RDM6300Helper::~RDM6300Helper()
{
    delete[] consecutiveReads;
    delete[] currentTags;
}

void RDM6300Helper::initRfids()
{
    for (uint16_t i = 0; i < sizeRfids; i++) {
        rfids[i].begin();
    }
}

bool RDM6300Helper::isValidTag(int idxReader)
{
    if (!currentTags[idxReader].length()) {
        return false;
    }

    for (uint16_t i = 0; i < sizeTags; i++) {
        String theTag = tags[(idxReader * sizeTags) + i];

        if (theTag.compareTo(currentTags[idxReader]) == 0) {
            return true;
        }
    }

    return false;
}

bool RDM6300Helper::shouldPoll()
{
    if (lastPoll == 0) {
        return true;
    }

    unsigned long now = millis();

    if (lastPoll > now) {
        // Millis overflow
        return true;
    }

    unsigned long diff = now - lastPoll;

    return diff >= pollMs;
}

void RDM6300Helper::readRfid(int idxReader)
{
    Serial.print(F("Reading #"));
    Serial.println(idxReader);

    String tagId = rfids[idxReader].getTagId();

    if (!tagId.length()) {
        Serial.println(F("Empty"));
        currentTags[idxReader] = "";
        consecutiveReads[idxReader] = 0;
        return;
    }

    bool isSame = currentTags[idxReader].compareTo(tagId) == 0;

    if (isSame) {
        consecutiveReads[idxReader]++;
    } else {
        consecutiveReads[idxReader] = 1;
        currentTags[idxReader] = tagId;
    }

    Serial.print(currentTags[idxReader]);
    Serial.print(F(" / "));
    Serial.println(consecutiveReads[idxReader]);
}

void RDM6300Helper::pollRfids()
{
    if (!shouldPoll()) {
        return;
    }

    for (uint16_t i = 0; i < sizeRfids; i++) {
        readRfid(i);
    }

    runCallbacks();

    lastPoll = millis();
}

void RDM6300Helper::addCallback(
    rdm6300_callback_func callback,
    uint16_t minReads = 1,
    int idxReader = -1,
    bool onlyOnce = true,
    bool anyTag = false)
{
    if (cbCounter >= MAX_RD6300_CALLBACKS) {
        return;
    }

    callbacks[cbCounter] = {
        .callback = callback,
        .minReads = minReads,
        .idxReader = idxReader,
        .onlyOnce = onlyOnce,
        .anyTag = anyTag
    };

    cbCounter++;
}

void RDM6300Helper::runCallback(int idxCb, int idxReader)
{
    Serial.print(F("Cb#"));
    Serial.print(idxCb);
    Serial.print(F(" R#"));
    Serial.println(idxReader);

    bool enabledForReader = callbacks[idxCb].idxReader == -1
        || callbacks[idxCb].idxReader == idxReader;

    if (!enabledForReader) {
        Serial.println(F("Not enabled for reader"));
        return;
    }

    bool enoughReads = consecutiveReads[idxReader] >= callbacks[idxCb].minReads;

    if (!enoughReads) {
        Serial.println(F("Not enough reads"));
        return;
    }

    bool alreadyRun = callbacks[idxCb].onlyOnce
        && consecutiveReads[idxReader] > callbacks[idxCb].minReads;

    if (alreadyRun) {
        Serial.println(F("Run already"));
        return;
    }

    bool enabledForTag = callbacks[idxCb].anyTag
        || isValidTag(idxReader);

    if (!enabledForTag) {
        Serial.println(F("Not enabled for tag"));
        return;
    }

    callbacks[idxCb].callback(idxReader, currentTags[idxReader]);
}

void RDM6300Helper::runCallbacks()
{
    for (int idxCb = 0; idxCb < cbCounter; idxCb++) {
        for (int idxReader = 0; idxReader < static_cast<int>(sizeRfids); idxReader++) {
            runCallback(idxCb, idxReader);
        }
    }
}
