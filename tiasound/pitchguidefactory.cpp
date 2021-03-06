/* TIATracker, (c) 2016 Andre "Kylearan" Wichmann.
 * Website: https://bitbucket.org/kylearan/tiatracker
 * Email: andre.wichmann@gmx.de
 * See the file "license.txt" for information on usage and redistribution
 * of this file.
 */

#include "pitchguidefactory.h"
#include "percussiontab.h"
#include <cmath>


namespace TiaSound {

PitchGuideFactory::PitchGuideFactory()
{
    // Generate lists of available frequencies for all TIA distortions
    for (int iDist = 0; iDist < PercussionTab::availableWaveforms.size(); ++iDist) {
        Distortion dist = PercussionTab::availableWaveforms[iDist];
        double divider = distDividers[dist];
        QList<double> palList;
        QList<double> ntscList;
        for (int f = 0; f < 32; ++f) {
            if (dist == Distortion::SILENT) {
                palList.append(0.0);
                ntscList.append(0.0);

            } else {
                palList.append(PalFrequency/divider/(1 + f));
                ntscList.append(NtscFrequency/divider/(1 + f));
            }
        }
        distFrequenciesPal[dist] = palList;
        distFrequenciesNtsc[dist] = ntscList;
    }
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getPitchPerfectPalGuide() {
    return calculateGuide("PAL Pitch-perfect A4=440Hz", TvStandard::PAL, 440.0);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getPitchPerfectNtscGuide() {
    return calculateGuide("NTSC Pitch-perfect A4=440Hz", TvStandard::NTSC, 440.0);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedPurePalGuide() {
    return calculateGuide("PAL Optimized Pure A4=615.4Hz", TvStandard::PAL, 615.427);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedPureNtscGuide() {
    return calculateGuide("NTSC Optimized Pure A4=585.3Hz", TvStandard::NTSC, 585.327);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv232PalGuide() {
    return calculateGuide("PAL Optimized Buzzy Rumble, A4=338.8Hz", TvStandard::PAL, 338.827);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv232NtscGuide() {
    return calculateGuide("NTSC Optimized Buzzy Rumble, A4=322.2Hz", TvStandard::NTSC, 322.227);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv93PalGuide() {
    return calculateGuide("PAL Optimized Electronic Rumble, A4=336.1Hz", TvStandard::PAL, 336.127);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv93NtscGuide() {
    return calculateGuide("NTSC Optimized Electronic Rumble, A4=319.7Hz", TvStandard::NTSC, 319.727);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv31PalGuide() {
    return calculateGuide("PAL Optimized Flangy/Wavering, Pure Buzzy, Reedy Rumble, White Noise, Electronic Squal, A4=317.6Hz", TvStandard::PAL, 317.627);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv31NtscGuide() {
    return calculateGuide("NTSC Optimized Flangy/Wavering, Pure Buzzy, Reedy Rumble, White Noise, Electronic Squal, A4=339.1Hz", TvStandard::NTSC, 339.127);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv15PalGuide() {
    return calculateGuide("PAL Optimized Buzzy, A4=328.2Hz", TvStandard::PAL, 328.227);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv15NtscGuide() {
    return calculateGuide("NTSC Optimized Buzzy, A4=330.7Hz", TvStandard::NTSC, 330.727);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv6PalGuide() {
    return calculateGuide("PAL Optimized Pure Low, A4=580.2Hz", TvStandard::PAL, 580.227);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv6NtscGuide() {
    return calculateGuide("NTSC Optimized Pure Low, A4=619.4Hz", TvStandard::NTSC, 619.427);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv2PalGuide() {
    return calculateGuide("PAL Optimized Pure High, A4=580.8Hz", TvStandard::PAL, 580.827);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::getOptimizedDiv2NtscGuide() {
    return calculateGuide("NTSC Optimized Pure High, A4=585.3Hz", TvStandard::NTSC, 585.327);
}

/*************************************************************************/

PitchGuide PitchGuideFactory::calculateGuide(QString name, TvStandard standard, double freq) {
    PitchGuide newGuide{name, standard, freq};
    for (int iDist = 0; iDist < PercussionTab::availableWaveforms.size(); ++iDist) {
        Distortion dist = PercussionTab::availableWaveforms[iDist];
        QList<FrequencyPitchGuide> guide = calcInstrumentPitchGuide(standard, dist, freq);
        newGuide.instrumentGuides[dist] = InstrumentPitchGuide(dist, newGuide.name, guide);
    }
    // Pure Combined has to be added manually
    QList<FrequencyPitchGuide> combinedGuide = calcInstrumentPitchGuide(standard, Distortion::PURE_HIGH, freq);
    combinedGuide.append(calcInstrumentPitchGuide(standard, Distortion::PURE_LOW, freq));
    newGuide.instrumentGuides[Distortion::PURE_COMBINED] = InstrumentPitchGuide(Distortion::PURE_COMBINED, newGuide.name, combinedGuide);
    return newGuide;
}

/*************************************************************************/

QList<FrequencyPitchGuide> PitchGuideFactory::calcInstrumentPitchGuide(TiaSound::TvStandard standard, Distortion dist, double baseFreq) {
    QList<FrequencyPitchGuide> fpg;
    // Get nearest notes for all TIA frequencies
    for (int f = 0; f < 32; ++f) {
        double tiaFreq;
        if (standard == TvStandard::PAL) {
            tiaFreq = distFrequenciesPal[dist][f];
        } else {
            tiaFreq = distFrequenciesNtsc[dist][f];
        }
        // Calc minimal distance of this frequency to all "real" notes
        double minPercent = 999999.0;
        int bestNoteIndex = 0;
        for (int n = 0; n < 9*12; ++n) {
            double noteFreq = std::pow(2.0, (n - 49.0)/12.0) * baseFreq;
            double deltaPercent = (std::log(tiaFreq/noteFreq)/std::log(2))*1200;
            if (std::abs(deltaPercent) < std::abs(minPercent)) {
                minPercent = deltaPercent;
                bestNoteIndex = n;
            }
        }
        // Store nearest note if it's close enough
        Note ttNote = Note::NotANote;
        if (minPercent < 50) {
            // For real notes, n=49. In TIATracker, it's 45
            int ttNoteIndex = bestNoteIndex - 4;
            // Only store notes we can display
            if (ttNoteIndex >= 0 && ttNoteIndex < numNotes) {
                ttNote = getNoteFromInt(ttNoteIndex);
            }
        } else {
            // So the pitch guide optimizer won't be confused
            minPercent = 0;
        }
        fpg.append(FrequencyPitchGuide{ttNote, int(std::round(minPercent))});
    }
    return fpg;
}

}


