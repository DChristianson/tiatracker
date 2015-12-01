#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <QString>
#include <QList>
#include "tiasound/tiasound.h"


namespace Track {

    class Instrument
    {
    public:
        Instrument(QString name) : name(name) {}

        QString name;
        TiaSound::Distortion baseDistortion{TiaSound::Distortion::BUZZY};
        QList<int> volumes{0, 0};
        QList<int> frequencies{0, 0};

        bool isEmpty();
        int getEnvelopeLength();
        void setEnvelopeLength(int newSize);
        void validateSustainReleaseValues();
        void setSustainAndRelease(int newSustainStart, int newReleaseStart);
        int getMinVolume();
        int getMaxVolume();
        int getSustainStart() const;
        int getReleaseStart() const;

    private:
        int envelopeLength = 2;
        int sustainStart = 0;
        int releaseStart = 1;
    };

}

#endif // INSTRUMENT_H
