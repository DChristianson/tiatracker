#include "trackcommand.h"

// SetRowToNoteTrackCommand

SetRowToNoteTrackCommand::SetRowToNoteTrackCommand(Track::Track* track, int patternIndex, int noteIndex, QUndoCommand *parent /*= nullptr*/) :
    TrackCommand(track, "", parent),
    iPatternIndex(patternIndex),
    iNoteIndex(noteIndex),
    oldNote(track->patterns[patternIndex].notes[noteIndex])
{
}

void SetRowToNoteTrackCommand::undo()
{
    pTrack->patterns[iPatternIndex].notes[iNoteIndex] = oldNote;
}

// SetRowToInstrumentTrackCommand

SetRowToInstrumentTrackCommand::SetRowToInstrumentTrackCommand(Track::Track* track, int patternIndex, int noteIndex, int instrumentIndex, int frequency,
    QUndoCommand *parent /*= nullptr*/) :
    SetRowToNoteTrackCommand(track, patternIndex, noteIndex, parent),
    iInstrumentIndex(instrumentIndex),
    iFrequency(frequency)
{

}

void SetRowToInstrumentTrackCommand::redo()
{
    pTrack->patterns[iPatternIndex].notes[iNoteIndex].type = Track::Note::instrumentType::Instrument;
    pTrack->patterns[iPatternIndex].notes[iNoteIndex].instrumentNumber = iInstrumentIndex;
    pTrack->patterns[iPatternIndex].notes[iNoteIndex].value = iFrequency;
}

// SetRowToPercussionTrackCommand

SetRowToPercussionTrackCommand::SetRowToPercussionTrackCommand(Track::Track* track, int patternIndex, int noteIndex, int percussionIndex,
    QUndoCommand *parent /*= nullptr*/) :
    SetRowToNoteTrackCommand(track, patternIndex, noteIndex, parent),
    iPercussionIndex(percussionIndex)
{

}

void SetRowToPercussionTrackCommand::redo()
{
    pTrack->patterns[iPatternIndex].notes[iNoteIndex].type = Track::Note::instrumentType::Percussion;
    pTrack->patterns[iPatternIndex].notes[iNoteIndex].instrumentNumber = iPercussionIndex;
}