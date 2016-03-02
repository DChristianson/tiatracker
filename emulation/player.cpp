#include <QThread>
#include <QTimer>
#include <iostream>
#include "track/track.h"
#include "track/instrument.h"
#include "player.h"
#include <SDL.h>
#include <QElapsedTimer>

namespace Emulation {

Player::Player(Track::Track *parentTrack, QObject *parent) : QThread(parent)
{
    pTrack = parentTrack;

    tiaSound.channels(2, false);
    sdlSound.setFrameRate(50.0);
    sdlSound.open();
    sdlSound.mute(false);
    sdlSound.setEnabled(true);
    sdlSound.setVolume(100);

    sdlSound.set(AUDC0, 0, 10);
    sdlSound.set(AUDV0, 0, 15);
    sdlSound.set(AUDF0, 0, 18);
}

Player::~Player()
{
    delete timer;
}

/*************************************************************************/

void Player::run() {
    timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timerFired()));
    timer->start(1000/50);
}

/*************************************************************************/

void Player::setFrameRate(float rate) {
    sdlSound.close();
    sdlSound.setFrameRate(rate);
    sdlSound.open();
    timer->setInterval(int(1000/rate));
}

/*************************************************************************/

void Player::silence() {
    mode = PlayMode::None;
}

/*************************************************************************/

void Player::playInstrument(Track::Instrument *instrument, int frequency) {
    currentInstrument = instrument;
    currentInstrumentFrequency = frequency;
    currentInstrumentFrame = 0;
    mode = PlayMode::Instrument;
}

/*************************************************************************/

void Player::playInstrumentOnce(Track::Instrument *instrument, int frequency) {
    currentInstrument = instrument;
    currentInstrumentFrequency = frequency;
    currentInstrumentFrame = 0;
    mode = PlayMode::InstrumentOnce;
}

/*************************************************************************/

void Player::stopInstrument() {
    currentInstrumentFrame = currentInstrument->getReleaseStart();
}

/*************************************************************************/

void Player::playPercussion(Track::Percussion *percussion) {
    currentPercussion = percussion;
    currentPercussionFrame = 0;
    mode = PlayMode::Percussion;
}

/*************************************************************************/

void Player::stopPercussion() {
    mode = PlayMode::None;
}

/*************************************************************************/

void Player::setChannel0(int distortion, int frequency, int volume) {
    sdlSound.set(AUDC0, distortion, 10);
    sdlSound.set(AUDV0, volume, 15);
    sdlSound.set(AUDF0, frequency, 18);
}

/*************************************************************************/

void Player::setChannel(int channel, int distortion, int frequency, int volume) {
    int audC = channel == 0 ? AUDC0 : AUDC1;
    int audV = channel == 0 ? AUDV0 : AUDV1;
    int audF = channel == 0 ? AUDF0 : AUDF1;
    sdlSound.set(audC, distortion, 10);
    sdlSound.set(audV, volume, 15);
    sdlSound.set(audF, frequency, 18);
}

/*************************************************************************/

void Player::playWaveform(TiaSound::Distortion waveform, int frequency, int volume) {
    int distortion = TiaSound::getDistortionInt(waveform);
    setChannel0(distortion, frequency, volume);
    mode = PlayMode::Waveform;
}

/*************************************************************************/

void Player::playTrack(int start1, int start2) {
    trackCurNoteIndex[0] = pTrack->getNoteIndexInPattern(0, start1);
    trackCurNoteIndex[1] = pTrack->getNoteIndexInPattern(1, start2);
    trackCurEntryIndex[0] = pTrack->getSequenceEntryIndex(0, start1);
    trackCurEntryIndex[1] = pTrack->getSequenceEntryIndex(0, start2);
    trackCurTick = 0;
    trackMode[0] = Track::Note::instrumentType::Hold;
    trackMode[1] = Track::Note::instrumentType::Hold;
    // InstrumentNumber -1 means: No note yet
    Track::Note startNote(Track::Note::instrumentType::Instrument, -1, -1);
    trackCurNote[0] = startNote;
    trackCurNote[1] = startNote;
    mode = PlayMode::Track;
}

/*************************************************************************/

void Player::updateSilence() {
    setChannel(0, 0, 0, 0);
    setChannel(1, 0, 0, 0);
}

/*************************************************************************/

void Player::updateInstrument() {
    /* Play current frame */
    // Check if instrument has changed and made currentFrame illegal
    pTrack->lock();
    if (currentInstrumentFrame >= currentInstrument->getEnvelopeLength()) {
        // Go into silence if currentFrame is illegal
        mode = PlayMode::None;
    } else {
        int CValue = currentInstrument->getAudCValue(currentInstrumentFrequency);
        int realFrequency = currentInstrumentFrequency <= 31 ? currentInstrumentFrequency : currentInstrumentFrequency - 32;
        int FValue = realFrequency + currentInstrument->frequencies[currentInstrumentFrame];
        // Check if envelope has caused an underrun
        if (FValue < 0) {
            FValue = 256 + FValue;
        }
        int VValue = currentInstrument->volumes[currentInstrumentFrame];
        setChannel0(CValue, FValue, VValue);

        /* Advance frame */
        currentInstrumentFrame++;
        // Check for end of sustain or release
        if (mode != PlayMode::InstrumentOnce
                && currentInstrumentFrame == currentInstrument->getReleaseStart()) {
            currentInstrumentFrame = currentInstrument->getSustainStart();
        } else if (currentInstrumentFrame == currentInstrument->getEnvelopeLength()) {
            // End of release: Go into silence
            mode = PlayMode::None;
        }
    }
    pTrack->unlock();
}

/*************************************************************************/

void Player::updatePercussion() {
    /* Play current frame */
    pTrack->lock();
    if (currentPercussionFrame >= currentPercussion->getEnvelopeLength()) {
        // Go into silence if currentFrame is illegal
        mode = PlayMode::None;
    } else {
        TiaSound::Distortion waveform = currentPercussion->waveforms[currentPercussionFrame];
        int CValue = TiaSound::getDistortionInt(waveform);
        int FValue = currentPercussion->frequencies[currentPercussionFrame];
        int VValue = currentPercussion->volumes[currentPercussionFrame];
        setChannel0(CValue, FValue, VValue);

        /* Advance frame. End of waveform is handled next frame */
        currentPercussionFrame++;
    }
    pTrack->unlock();
}

/*************************************************************************/

void Player::sequenceChannel(int channel) {
    if (mode == PlayMode::None) {
        return;
    }
    // Get next note
    if (!pTrack->getNextNoteWithGoto(channel, &(trackCurEntryIndex[channel]), &(trackCurNoteIndex[channel]))) {
        mode = PlayMode::None;
    }
    int patternIndex = pTrack->channelSequences[channel].sequence[trackCurEntryIndex[channel]].patternIndex;
    Track::Note *nextNote = &(pTrack->patterns[patternIndex].notes[trackCurNoteIndex[channel]]);
    // Parse and validate next note
    switch(nextNote->type) {
    case Track::Note::instrumentType::Hold:
        break;
    case Track::Note::instrumentType::Instrument:
        trackMode[channel] = Track::Note::instrumentType::Instrument;
        trackCurNote[channel] = *nextNote;
        trackCurEnvelopeIndex[channel] = 0;
        break;
    case Track::Note::instrumentType::Pause:
        if (trackMode[channel] != Track::Note::instrumentType::Instrument) {
            mode = PlayMode::None;
            updateSilence();
            std::cout << "A\n"; std::cout.flush();
            emit invalidNoteFound(channel, trackCurEntryIndex[channel], trackCurNoteIndex[channel]);
        } else {
            trackMode[channel] = Track::Note::instrumentType::Hold;
            Track::Instrument *curInstrument = &(pTrack->instruments[trackCurNote[channel].instrumentNumber]);
            trackCurEnvelopeIndex[channel] = curInstrument->getSustainStart();
        }
        break;
    case Track::Note::instrumentType::Percussion:
        trackMode[channel] = Track::Note::instrumentType::Percussion;
        trackCurNote[channel] = *nextNote;
        trackCurEnvelopeIndex[channel] = 0;
        break;
    case Track::Note::instrumentType::Slide:
        if (trackMode[channel] != Track::Note::instrumentType::Instrument) {
            mode = PlayMode::None;
            updateSilence();
            std::cout << "B\n"; std::cout.flush();
            emit invalidNoteFound(channel, trackCurEntryIndex[channel], trackCurNoteIndex[channel]);
        } else {
            trackCurNote[channel].value += nextNote->value;
            // Check for over-/underflow
            if (trackCurNote[channel].value < 0
                    || (trackCurNote[channel].value)%32 > 31) {
                mode = PlayMode::None;
                updateSilence();
                std::cout << "C\n"; std::cout.flush();
                emit invalidNoteFound(channel, trackCurEntryIndex[channel], trackCurNoteIndex[channel]);
            }
        }
        break;
    }
}

/*************************************************************************/

void Player::updateChannel(int channel) {
    if (mode == PlayMode::None) {
        return;
    }
    // Play current note
    if (trackCurNote[channel].instrumentNumber != -1) {
        switch(trackCurNote[channel].type) {
        case Track::Note::instrumentType::Instrument:
        {
            Track::Instrument *curInstrument = &(pTrack->instruments[trackCurNote[channel].instrumentNumber]);
            int CValue = curInstrument->getAudCValue(trackCurNote[channel].value);
            // If at end of release, play silence; otherwise ADSR envelope
            if (trackCurEnvelopeIndex[channel] >= curInstrument->getEnvelopeLength()) {
                setChannel(channel, CValue, 0, 0);
            } else {
                int curFrequency = trackCurNote[channel].value;
                int realFrequency = curFrequency <= 31 ? curFrequency : curFrequency - 32;
                int FValue = realFrequency + curInstrument->frequencies[trackCurEnvelopeIndex[channel]];
                // Check if envelope has caused an underrun
                if (FValue < 0) {
                    FValue = 256 + FValue;
                }
                int VValue = curInstrument->volumes[trackCurEnvelopeIndex[channel]];
                setChannel(channel, CValue, FValue, VValue);
                // Advance frame
                trackCurEnvelopeIndex[channel]++;
                // Check for end of sustain
                if (trackCurEnvelopeIndex[channel] == curInstrument->getReleaseStart()) {
                    trackCurEnvelopeIndex[channel] = curInstrument->getSustainStart();
                }
            }
            break;
        }
        case Track::Note::instrumentType::Percussion:
        {
            Track::Percussion *curPercussion = &(pTrack->percussion[trackCurNote[channel].instrumentNumber]);
            if (trackCurEnvelopeIndex[channel] < curPercussion->getEnvelopeLength()) {
                TiaSound::Distortion waveform = curPercussion->waveforms[trackCurEnvelopeIndex[channel]];
                int CValue = TiaSound::getDistortionInt(waveform);
                int FValue = curPercussion->frequencies[trackCurEnvelopeIndex[channel]];
                int VValue = curPercussion->volumes[trackCurEnvelopeIndex[channel]];
                setChannel(channel, CValue, FValue, VValue);
                // Advance frame
                trackCurEnvelopeIndex[channel]++;
            } else {
                // TODO: Handle overlay
                // Get last AUDC value, to mimick play routine
                TiaSound::Distortion waveform = curPercussion->waveforms[trackCurEnvelopeIndex[channel] - 1];
                int CValue = TiaSound::getDistortionInt(waveform);
                setChannel(channel, CValue, 0, 0);
            }
            break;
        }
        default:
            break;
        }
    }
}

/*************************************************************************/

void Player::updateTrack() {
    pTrack->lock();
    if (--trackCurTick < 0) {
        sequenceChannel(0);
        sequenceChannel(1);
        trackCurTick = trackCurNoteIndex[0]%2 == 0 ? pTrack->oddSpeed : pTrack->evenSpeed;
        int pos1 = pTrack->channelSequences[0].sequence[trackCurEntryIndex[0]].firstNoteNumber
                + trackCurNoteIndex[0];
        int pos2 = pTrack->channelSequences[1].sequence[trackCurEntryIndex[1]].firstNoteNumber
                + trackCurNoteIndex[1];
        emit newPlayerPos(pos1, pos2);
    }
    updateChannel(0);
    updateChannel(1);
    pTrack->unlock();
}

/*************************************************************************/

void Player::timerFired() {
    switch (mode) {
    case PlayMode::Instrument:
    case PlayMode::InstrumentOnce:
        updateInstrument();
        break;
    case PlayMode::Waveform:
        break;
    case PlayMode::Percussion:
        updatePercussion();
        break;
    case PlayMode::Track:
        updateTrack();
        break;
    default:
        updateSilence();
    }
}

}
