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

class SetSlideValueCommand : public SetRowToNoteCommand
{
    int iSlideValue;

public:
    SetSlideValueCommand(Track::Track* track, int patternIndex, int noteIndex, int slideValue);

    Track::Note newNote() const final;
};

class SetHoldCommand : public SetRowToNoteCommand
{
public:
    SetHoldCommand(Track::Track* track, int patternIndex, int noteIndex);

    Track::Note newNote() const final;
};

class SetPauseCommand : public SetRowToNoteCommand
{
public:
    SetPauseCommand(Track::Track* track, int patternIndex, int noteIndex);

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
    int iEntryIndex;

public:
    InsertPatternCommand(Track::Track* track, int patternIndex, int channel, int entryIndex,
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
    int iEntryIndex;
    QString sPatternName;

public:
    DuplicatePatternCommand(Track::Track* track, int patternIndex, int channel, int entryIndex, const QString& patternName);

    void do_undo() final;
    void do_redo() final;
};

class RemovePatternCommand : public TrackCommand
{
    int iPatternIndex;
    int iChannel;
    int iEntryIndex;
    bool bDeletePattern;
    Track::Pattern pattern;

public:
    RemovePatternCommand(Track::Track* track, int patternIndex, int channel, int entryIndex, bool deletePattern);

    void do_undo() final;
    void do_redo() final;
};

class RenamePatternCommand : public TrackCommand
{
    int iPatternIndex;
    QString sNewPatternName;
    QString sOldPatternName;

public:
    RenamePatternCommand(Track::Track* track, int patternIndex, const QString& patternName);

    void do_undo() final;
    void do_redo() final;
};

class SetGotoCommand : public TrackCommand
{
    int iChannel;
    int iEntryIndex;
    int iNewGoto;
    int iOldGoto;

public:
    SetGotoCommand(Track::Track* track, int channel, int entryIndex, int gotoTarget);

    void do_undo() final;
    void do_redo() final;
};

class SetStartPatternCommand : public TrackCommand
{
    int iChannel;
    int iNewStartPattern;
    int iOldStartPattern;

public:
    SetStartPatternCommand(Track::Track* track, int channel, int startPattern);

    void do_undo() final;
    void do_redo() final;
};

class DeleteRowCommand : public TrackCommand
{
    int iPatternIndex;
    int iNoteIndex;
    Track::Note oldNote;

public:
    DeleteRowCommand(Track::Track* track, int patternIndex, int noteIndex);

    void do_undo() final;
    void do_redo() final;
};

class InsertRowCommand : public TrackCommand
{
    int iPatternIndex;
    int iNoteIndex;

public:
    InsertRowCommand(Track::Track* track, int patternIndex, int noteIndex);

    void do_undo() final;
    void do_redo() final;
};

template<typename T>
class SetValueCommand : public TrackCommand
{
protected:
    T& value;
    T  oldValue;
    T  newValue;
public:
    SetValueCommand(Track::Track* track, T& v, const T newV, QUndoCommand* parent = nullptr) :
        TrackCommand(track, "", parent),
        value(v),
        oldValue(v),
        newValue(newV)
    {
    }

    void do_undo() final { value = oldValue; }
    void do_redo() final { value = newValue; }
};

class SetStringCommand : public SetValueCommand<QString>
{
    int _id;
    static bool ID;
public:
    SetStringCommand(Track::Track* track, QString& v, const QString newV):
        SetValueCommand<QString>(track, v, newV) { _id = int(ID); }

    int id() const final { return _id; }

    bool mergeWith(const QUndoCommand *other) final
    {
        if (other->id() != id())
            return false;
        auto other_like_me = dynamic_cast<const SetStringCommand*>(other);
        assert(other_like_me != nullptr);
        if (&value != &other_like_me->value)
            return false; // target mismatch
        newValue = other_like_me->newValue;
        if (newValue == oldValue)
            setObsolete(true);
        return true; // other will be deleted
    }

    static void ToggleID() { ID = !ID; }
};

class SetPatternSpeedCommand : public TrackCommand
{
    int iPatternIndex;
    bool bEven;
    int newSpeed;
    int oldSpeed;

public:
    SetPatternSpeedCommand(Track::Track* track, int patternIndex, bool even, int speed);

    void do_undo() final;
    void do_redo() final;
};

// IP: Instrument or Percussion
// ids for mergeable commands:
//    - ids 0 and 1 are for SetStringCommand
//    - ids 2 and 3 are for SetInstrumentCommand
//    - ids 4 and 5 are for SetPercussionCommand
template<class IP> 
class SetAbstractInstrumentCommand : public TrackCommand
{
protected:
    int iInstrumentIndex;
    IP newInstrument;
    IP oldInstrument;

private:
    bool bMergeable;

    // mutable and const below for lazy init of _id
    mutable int _id;
    virtual int init_id() const = 0;

public:
    SetAbstractInstrumentCommand(Track::Track* track, int instrumentIndex, IP&& instrument, const IP& currInstrument,
        bool mergeable) :
        TrackCommand(track, "", nullptr),
        iInstrumentIndex(instrumentIndex),
        newInstrument(instrument),
        oldInstrument(currInstrument),
        bMergeable(mergeable),
        _id(-1)
    {
    }

    int id() const final {
        if (!bMergeable)
            return -1;
        if (_id == -1)
            _id = init_id(); // lazy init of _id because we can't call init_id from the ctor
        return _id;
    }

    bool mergeWith(const QUndoCommand *other) final
    {
        if (other->id() != id())
            return false;
        auto other_like_me = dynamic_cast<const SetAbstractInstrumentCommand<IP>*>(other);
        assert(other_like_me != nullptr);
        if (iInstrumentIndex != other_like_me->iInstrumentIndex)
            return false; // target mismatch
        newInstrument = other_like_me->newInstrument;
        if (newInstrument == oldInstrument)
            setObsolete(true);
        return true; // other will be deleted
    }
};

class SetInstrumentCommand : public SetAbstractInstrumentCommand<Track::Instrument>
{
    static bool ID;

    int init_id() const final {
        return int(ID) + 2;
    }

public:
    SetInstrumentCommand(Track::Track* track, int instrumentIndex, Track::Instrument&& instrument,
        bool mergeable = false) :
        SetAbstractInstrumentCommand<Track::Instrument>(track, instrumentIndex, std::move(instrument), track->instruments[instrumentIndex], mergeable)
    {}

    void do_undo() final;
    void do_redo() final;

    static void ToggleID() { ID = !ID; }
};

class SetPercussionCommand : public SetAbstractInstrumentCommand<Track::Percussion>
{
    static bool ID;

    int init_id() const final {
        return int(ID) + 4;
    }

public:
    SetPercussionCommand(Track::Track* track, int percussionIndex, Track::Percussion&& percussion,
        bool mergeable = false) :
        SetAbstractInstrumentCommand<Track::Percussion>(track, percussionIndex, std::move(percussion), track->percussion[percussionIndex], mergeable)
    {}

    void do_undo() final;
    void do_redo() final;

    static void ToggleID() { ID = !ID; }
};

#endif // TRACKCOMMAND_H
