#include "trackcommand.h"

// SetRowToNoteCommand

SetRowToNoteCommand::SetRowToNoteCommand(Track::Track* track, int patternIndex, int noteIndex) :
    TrackCommand(track, "", nullptr),
    iPatternIndex(patternIndex),
    iNoteIndex(noteIndex),
    oldNote(track->patterns[patternIndex].notes[noteIndex])
{
}

void SetRowToNoteCommand::do_undo()
{
    pTrack->patterns[iPatternIndex].notes[iNoteIndex] = oldNote;
}

void SetRowToNoteCommand::do_redo()
{
    pTrack->patterns[iPatternIndex].notes[iNoteIndex] = newNote();
}

// SetRowToInstrumentCommand

SetRowToInstrumentCommand::SetRowToInstrumentCommand(Track::Track* track, int patternIndex, int noteIndex, int instrumentIndex, int frequency) :
    SetRowToNoteCommand(track, patternIndex, noteIndex),
    iInstrumentIndex(instrumentIndex),
    iFrequency(frequency)
{

}

Track::Note SetRowToInstrumentCommand::newNote() const
{
    return { Track::Note::instrumentType::Instrument, iInstrumentIndex, iFrequency };
}

// SetRowToPercussionCommand

SetRowToPercussionCommand::SetRowToPercussionCommand(Track::Track* track, int patternIndex, int noteIndex, int percussionIndex) :
    SetRowToNoteCommand(track, patternIndex, noteIndex),
    iPercussionIndex(percussionIndex)
{

}

Track::Note SetRowToPercussionCommand::newNote() const
{
    return { Track::Note::instrumentType::Percussion, iPercussionIndex, 0/*unused*/ };
}

// SetSlideValueCommand

SetSlideValueCommand::SetSlideValueCommand(Track::Track* track, int patternIndex, int noteIndex, int slideValue) :
    SetRowToNoteCommand(track, patternIndex, noteIndex),
    iSlideValue(slideValue)
{

}

Track::Note SetSlideValueCommand::newNote() const
{
    return { Track::Note::instrumentType::Slide, 0/*unused*/, iSlideValue };
}

// CreatePatternCommand

CreatePatternCommand::CreatePatternCommand(Track::Track* track, const Track::Pattern& newPattern, QUndoCommand *parent) :
    TrackCommand(track, "", parent),
    pattern(newPattern)
{

}

void CreatePatternCommand::do_undo()
{
    pTrack->patterns.removeLast();
}

void CreatePatternCommand::do_redo()
{
    pTrack->patterns.append(pattern);
}

// InsertPatternCommand

InsertPatternCommand::InsertPatternCommand(Track::Track* track, int patternIndex, int channel, int entryIndex,
    QUndoCommand *parent /*= nullptr*/) :
    TrackCommand(track, "", parent),
    iPatternIndex(patternIndex),
    iChannel(channel),
    iEntryIndex(entryIndex)
{
}

void InsertPatternCommand::do_undo()
{
    pTrack->channelSequences[iChannel].sequence.removeAt(iEntryIndex);
    pTrack->updateFirstNoteNumbers();
}

void InsertPatternCommand::do_redo()
{
    Track::SequenceEntry newEntry(iPatternIndex);
    pTrack->channelSequences[iChannel].sequence.insert(iEntryIndex, newEntry);
    pTrack->updateFirstNoteNumbers();
}

// MovePatternCommand

MovePatternCommand::MovePatternCommand(Track::Track* track, int channel, int entryIndexFrom, int entryIndexTo) :
    TrackCommand(track, "", nullptr),
    iChannel(channel),
    iEntryIndexFrom(entryIndexFrom),
    iEntryIndexTo(entryIndexTo)
{
}

void MovePatternCommand::do_undo()
{
    pTrack->channelSequences[iChannel].sequence.swap(iEntryIndexTo, iEntryIndexFrom);
    pTrack->updateFirstNoteNumbers();
}

void MovePatternCommand::do_redo()
{
    pTrack->channelSequences[iChannel].sequence.swap(iEntryIndexFrom, iEntryIndexTo);
    pTrack->updateFirstNoteNumbers();
}

// DuplicatePatternCommand

DuplicatePatternCommand::DuplicatePatternCommand(Track::Track* track, int patternIndex, int channel, int entryIndex,
    const QString& patternName) :
    TrackCommand(track, "", nullptr),
    iPatternIndex(patternIndex),
    iChannel(channel),
    iEntryIndex(entryIndex),
    sPatternName(patternName)
{
}

void DuplicatePatternCommand::do_undo()
{
    pTrack->channelSequences[iChannel].sequence.removeAt(iEntryIndex + 1);
    pTrack->patterns.removeLast();
    pTrack->updateFirstNoteNumbers();
}

void DuplicatePatternCommand::do_redo()
{
    pTrack->patterns.append(pTrack->patterns[iPatternIndex]);
    pTrack->patterns.last().name = sPatternName;
    Track::SequenceEntry newEntry(pTrack->patterns.size() - 1);
    pTrack->channelSequences[iChannel].sequence.insert(iEntryIndex + 1, newEntry);
    pTrack->updateFirstNoteNumbers();
}

// RemovePatternCommand

RemovePatternCommand::RemovePatternCommand(Track::Track* track, int patternIndex, int channel, int entryIndex, bool deletePattern) :
    TrackCommand(track, "", nullptr),
    iPatternIndex(patternIndex),
    iChannel(channel),
    iEntryIndex(entryIndex),
    bDeletePattern(deletePattern),
    pattern()
{
    if (bDeletePattern)
        pattern = pTrack->patterns[iPatternIndex];
}

void RemovePatternCommand::do_undo()
{
    // Validate gotos
    for (int i = 0; i < pTrack->channelSequences[iChannel].sequence.size(); ++i) {
        if (pTrack->channelSequences[iChannel].sequence[i].gotoTarget+1 >= iEntryIndex) {
            pTrack->channelSequences[iChannel].sequence[i].gotoTarget++;
        }
    }

    // Create pattern
    if (bDeletePattern) {
        pTrack->patterns.insert(iPatternIndex, pattern);
    }

    // Insert pattern
    pTrack->channelSequences[iChannel].sequence.insert(iEntryIndex, iPatternIndex);
    pTrack->updateFirstNoteNumbers();

}

void RemovePatternCommand::do_redo()
{
    // Remove pattern
    pTrack->channelSequences[iChannel].sequence.removeAt(iEntryIndex);
    pTrack->updateFirstNoteNumbers();

    // Delete pattern if asked for
    if (bDeletePattern) {
        // Correct SequenceEntries
        for (int channel = 0; channel < 2; ++channel) {
            for (int i = 0; i < pTrack->channelSequences[channel].sequence.size(); ++i) {
                if (pTrack->channelSequences[channel].sequence[i].patternIndex > iPatternIndex) {
                    pTrack->channelSequences[channel].sequence[i].patternIndex--;
                }
            }
        }
        pTrack->patterns.removeAt(iPatternIndex);
    }

    // Validate gotos
    for (int i = 0; i < pTrack->channelSequences[iChannel].sequence.size(); ++i) {
        if (pTrack->channelSequences[iChannel].sequence[i].gotoTarget >= iEntryIndex) {
            pTrack->channelSequences[iChannel].sequence[i].gotoTarget--;
        }
    }
}

// RenamePatternCommand

RenamePatternCommand::RenamePatternCommand(Track::Track* track, int patternIndex, const QString& patternName) :
    TrackCommand(track, "", nullptr),
    iPatternIndex(patternIndex),
    sNewPatternName(patternName),
    sOldPatternName(pTrack->patterns[patternIndex].name)
{

}

void RenamePatternCommand::do_undo()
{
    pTrack->patterns[iPatternIndex].name = sOldPatternName;
}

void RenamePatternCommand::do_redo()
{
    pTrack->patterns[iPatternIndex].name = sNewPatternName;
}

// SetGotoCommand

SetGotoCommand::SetGotoCommand(Track::Track* track, int channel, int entryIndex, int gotoTarget) :
    TrackCommand(track, "", nullptr),
    iChannel(channel),
    iEntryIndex(entryIndex),
    iNewGoto(gotoTarget),
    iOldGoto(pTrack->channelSequences[iChannel].sequence[iEntryIndex].gotoTarget)
{
}

void SetGotoCommand::do_undo()
{
    Track::SequenceEntry *entry = &(pTrack->channelSequences[iChannel].sequence[iEntryIndex]);

    pTrack->lock();
    entry->gotoTarget = iOldGoto;
    pTrack->unlock();
}

void SetGotoCommand::do_redo()
{
    Track::SequenceEntry *entry = &(pTrack->channelSequences[iChannel].sequence[iEntryIndex]);

    pTrack->lock();
    entry->gotoTarget = iNewGoto;
    pTrack->unlock();
}

// SetStartPatternCommand

SetStartPatternCommand::SetStartPatternCommand(Track::Track* track, int channel, int startPattern) :
    TrackCommand(track, "", nullptr),
    iChannel(channel),
    iNewStartPattern(startPattern),
    iOldStartPattern(pTrack->startPatterns[iChannel])
{
}

void SetStartPatternCommand::do_undo()
{
    pTrack->lock();
    pTrack->startPatterns[iChannel] = iOldStartPattern;
    pTrack->unlock();
}

void SetStartPatternCommand::do_redo()
{
    pTrack->lock();
    pTrack->startPatterns[iChannel] = iNewStartPattern;
    pTrack->unlock();
}