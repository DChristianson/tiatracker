#ifndef TRACKCOMMAND_H
#define TRACKCOMMAND_H

#include <QUndoCommand>

#include "track/track.h"
#include "track/note.h"

#include "undostep.h"

class TrackCommand : public QUndoCommand
{
public:
    commandInfo ci;

protected:
    Track::Track* pTrack;

    void pre_emit(bool undo) {
        if (pre)
            pre->emit step(undo, text(), ci);
    }

    void post_emit(bool undo) {
        if (post)
            post->emit step(undo, text(), ci);
    }

    virtual void pre_undo() { pre_emit(true); }
    virtual void pre_redo() { pre_emit(false); }
    virtual void post_undo() { post_emit(true); }
    virtual void post_redo() { post_emit(false); }

public:
    UndoStep* pre = nullptr;
    UndoStep* post = nullptr;

    TrackCommand(Track::Track* track, const QString &text, QUndoCommand *parent = nullptr)
        :QUndoCommand(text, parent), pTrack(track) {}
    
    virtual void do_undo() { QUndoCommand::undo(); }
    virtual void do_redo() { QUndoCommand::redo(); }

    void undo() final { pre_undo(); do_undo(); post_undo(); }
    void redo() final { pre_redo(); do_redo(); post_redo(); }
};

class SetRowToNoteCommand : public TrackCommand
{
    Track::Note oldNote;

protected:
    int iPatternIndex;
    int iNoteIndex;

public:
    SetRowToNoteCommand(Track::Track* track, int patternIndex, int noteIndex);

    virtual Track::Note newNote() const = 0;

    void do_undo() final;
    void do_redo() final;
};

class SetRowToInstrumentCommand : public SetRowToNoteCommand
{
    int iInstrumentIndex;
    int iFrequency;

public:
    SetRowToInstrumentCommand(Track::Track* track, int patternIndex, int noteIndex, int instrumentIndex, int frequency);

    Track::Note newNote() const final;
};

class SetRowToPercussionCommand : public SetRowToNoteCommand
{
    int iPercussionIndex;

public:
    SetRowToPercussionCommand(Track::Track* track, int patternIndex, int noteIndex, int percussionIndex);

    Track::Note newNote() const final;
};


class CreatePatternCommand : public TrackCommand
{
    Track::Pattern pattern;

public:
    CreatePatternCommand(Track::Track* track, const Track::Pattern& newPattern, QUndoCommand *parent);

    void do_undo() final;
    void do_redo() final;
};

class InsertPatternCommand : public TrackCommand
{
    int iPatternIndex;
    int iChannel;
    int iNoteIndex;
    int iEntryIndex;

public:
    InsertPatternCommand(Track::Track* track, int patternIndex, int channel, int noteIndex, int entryIndex,
        QUndoCommand *parent = nullptr);

    void do_undo() final;
    void do_redo() final;
};

class MovePatternCommand : public TrackCommand
{
    int iChannel;
    int iEntryIndexFrom;
    int iEntryIndexTo;

public:
    MovePatternCommand(Track::Track* track, int channel, int entryIndexFrom, int entryIndexTo);

    void do_undo() final;
    void do_redo() final;
};

class DuplicatePatternCommand : public TrackCommand
{
    int iPatternIndex;
    int iChannel;
    int iNoteIndex;
    int iEntryIndex;
    QString sPatternName;

public:
    DuplicatePatternCommand(Track::Track* track, int patternIndex, int channel, int noteIndex, int entryIndex, const QString& patternName);

    void do_undo() final;
    void do_redo() final;
};

#endif // TRACKCOMMAND_H
