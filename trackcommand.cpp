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

InsertPatternCommand::InsertPatternCommand(Track::Track* track, bool doBefore, int patternIndex, int channel, int noteIndex,
    QUndoCommand *parent /*= nullptr*/) :
    TrackCommand(track, "", parent),
    iPatternIndex(patternIndex),
    iChannel(channel),
    iNoteIndex(noteIndex),
    iEntryIndex(pTrack->getSequenceEntryIndex(iChannel, iNoteIndex)+doBefore?0:1)
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
