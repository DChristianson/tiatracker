#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QThread>
#include <QTimer>

#include "track/track.h"
#include "track/instrument.h"
#include "tiasound/tiasound.h"
#include "emulation/TIASnd.h"
#include "emulation/SoundSDL2.h"
#include <QElapsedTimer>


namespace Emulation {

class Player : public QThread {
    Q_OBJECT

public:
    explicit Player(Track::Track *parentTrack, QObject *parent = 0);
    ~Player();

    /* Start the thread, i.e. initialize */
    void run();

    /* Set framerate to play at */
    void setFrameRate(float rate);

public slots:
    // Stop everything next frame
    void silence();

    /* Start to play a note with a given instrument */
    void playInstrument(Track::Instrument *instrument, int frequency);
    void playInstrumentOnce(Track::Instrument *instrument, int frequency);
    /* Stop playing a note, i.e. send instrument into release */
    void stopInstrument();

    /* Start and stop playing a percussion */
    void playPercussion(Track::Percussion *percussion);
    void stopPercussion();

    /* Start waveform */
    void playWaveform(TiaSound::Distortion waveform, int frequency, int volume);

signals:

private:
    Track::Track *pTrack = nullptr;
    Emulation::TIASound tiaSound;
    Emulation::SoundSDL2 sdlSound{&tiaSound};

    QTimer *timer = nullptr;

    // Play mode we are in
    enum class PlayMode {
        None, Instrument, InstrumentOnce, Percussion, Waveform
    };
    PlayMode mode = PlayMode::None;

    /* Current values for instrument play */
    Track::Instrument *currentInstrument;
    int currentInstrumentFrequency;
    int currentInstrumentFrame;

    /* Current values for percussion play */
    Track::Percussion *currentPercussion;
    int currentPercussionFrame;

    /* Helper methods for timerFired() */
    void updateSilence();
    void updateInstrument();
    void updatePercussion();

    /* Set values for channel 0 */
    void setChannel0(int distortion, int frequency, int volume);

private slots:
    void timerFired();
};

}

#endif // PLAYER_H
