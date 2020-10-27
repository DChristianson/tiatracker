#ifndef TRACKCOMMAND_H
#define TRACKCOMMAND_H

#include <QUndoCommand>

#include "track/track.h"
#include "track/note.h"

class TrackCommand : public QUndoCommand
{
protected:
    Track::Track* pTrack;

public:
    TrackCommand(Track::Track* track, const QString &text, QUndoCommand *parent = nullptr) :QUndoCommand(text, parent), pTrack(track) {}
};

class SetRowToNoteTrackCommand : public TrackCommand
{
    Track::Note oldNote;

protected:
    int iPatternIndex;
    int iNoteIndex;

public:
    SetRowToNoteTrackCommand(Track::Track* track, int patternIndex, int noteIndex, QUndoCommand *parent = nullptr);

    void undo() override;
};

class SetRowToInstrumentTrackCommand : public SetRowToNoteTrackCommand
{
    int iInstrumentIndex;
    int iFrequency;

public:
    explicit SetRowToInstrumentTrackCommand(Track::Track* track, int patternIndex, int noteIndex, int instrumentIndex, int frequency,
        QUndoCommand *parent = nullptr);

    void redo() override;
};

class SetRowToPercussionTrackCommand : public SetRowToNoteTrackCommand
{
    int iPercussionIndex;

public:
    explicit SetRowToPercussionTrackCommand(Track::Track* track, int patternIndex, int noteIndex, int percussionIndex,
        QUndoCommand *parent = nullptr);

    void redo() override;
};

#endif // TRACKCOMMAND_H
