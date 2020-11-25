/* TIATracker, (c) 2016 Andre "Kylearan" Wichmann.
 * Website: https://bitbucket.org/kylearan/tiatracker
 * Email: andre.wichmann@gmx.de
 * See the file "license.txt" for information on usage and redistribution
 * of this file.
 */

#include "tracktab.h"
#include <QPushButton>
#include <QLayout>
#include <QButtonGroup>
#include <iostream>
#include "instrumentselector.h"
#include "timeline.h"
#include "patterneditor.h"
#include <QLabel>
#include <QSpinBox>
#include "setslidedialog.h"
#include <QMessageBox>
#include "setfrequencydialog.h"
#include "instrumentstab.h"
#include "renamepatterndialog.h"
#include "setgotodialog.h"
#include "mainwindow.h"
#include "insertpatterndialog.h"
#include "createpatterndialog.h"
#include <qcheckbox.h>


TrackTab::TrackTab(QWidget *parent) : QWidget(parent)
{
    // Global
    addShortcut(&actionMoveUp, "CursorUp");
    addShortcut(&actionMoveDown, "CursorDown");
    addShortcut(&actionLeftChannel, "CursorLeftChannel");
    addShortcut(&actionRightChannel, "CursorRightChannel");
    addShortcut(&actionSwitchChannel, "CursorSwitchChannel");
    addShortcut(&actionFirstRow, "CursorFirstRow");
    addShortcut(&actionLastRow, "CursorLastRow");
    addShortcut(&actionNextPattern, "CursorNextPattern");
    addShortcut(&actionPreviousPattern, "CursorPreviousPattern");

    // Pattern
    addShortcut(&actionInsertPatternBefore, "PatternInsertBefore");
    addShortcut(&actionInsertPatternAfter, "PatternInsertAfter");
    patternContextMenu.addAction(&actionInsertPatternBefore);
    patternContextMenu.addAction(&actionInsertPatternAfter);
    addShortcut(&actionMovePatternUp, "PatternMoveUp");
    patternContextMenu.addAction(&actionMovePatternUp);
    addShortcut(&actionMovePatternDown, "PatternMoveDown");
    patternContextMenu.addAction(&actionMovePatternDown);
    addShortcut(&actionDuplicatePattern, "PatternDuplicate");
    patternContextMenu.addAction(&actionDuplicatePattern);
    addShortcut(&actionRemovePattern, "PatternRemove");
    patternContextMenu.addAction(&actionRemovePattern);
    addShortcut(&actionRenamePattern, "PatternRename");
    patternContextMenu.addAction(&actionRenamePattern);
    patternContextMenu.addSeparator();
    addShortcut(&actionSetGoto, "PatternSetGoto");
    patternContextMenu.addAction(&actionSetGoto);
    addShortcut(&actionRemoveGoto, "PatternRemoveGoto");
    patternContextMenu.addAction(&actionRemoveGoto);
    patternContextMenu.addAction(&actionSetStartPattern);
    // Channel
    addShortcut(&actionPause, "NotePause");
    channelContextMenu.addAction(&actionPause);
    addShortcut(&actionHold, "NoteHold");
    channelContextMenu.addAction(&actionHold);
    addShortcut(&actionSlide, "NoteSlide");
    channelContextMenu.addAction(&actionSlide);
    addShortcut(&actionSetFrequency, "NoteFrequency");
    channelContextMenu.addAction(&actionSetFrequency);
    channelContextMenu.addSeparator();
    addShortcut(&actionInsertRowBefore, "RowInsert");
    channelContextMenu.addAction(&actionInsertRowBefore);
    channelContextMenu.addAction(&actionInsertRowAfter);
    addShortcut(&actionDeleteRow, "RowDelete");
    channelContextMenu.addAction(&actionDeleteRow);
    channelContextMenu.addSeparator();
    actionMuteChannel.setCheckable(true);
    addShortcut(&actionMuteChannel, "ChannelToggleMute");
    channelContextMenu.addAction(&actionMuteChannel);
}

/*************************************************************************/

void TrackTab::registerTrack(Track::Track *newTrack) {
    pTrack = newTrack;
}

/*************************************************************************/

void TrackTab::registerPlayer(Emulation::Player *newPlayer) {
    pPlayer = newPlayer;
}

/*************************************************************************/

void TrackTab::initTrackTab() {
    pTrack->updateFirstNoteNumbers();

    InstrumentSelector *insSel = findChild<InstrumentSelector *>("trackInstrumentSelector");
    insSel->registerTrack(pTrack);
    insSel->initSelector();
    Timeline *timeline = findChild<Timeline *>("trackTimeline");
    timeline->registerTrack(pTrack);
    PatternEditor *editor = findChild<PatternEditor *>("trackEditor");
    editor->registerTrack(pTrack);
    editor->registerMuteAction(&actionMuteChannel);
    editor->registerInstrumentSelector(insSel);

    // Global actions
    QObject::connect(&actionMoveUp, SIGNAL(triggered(bool)), editor, SLOT(moveUp(bool)));
    QObject::connect(&actionMoveDown, SIGNAL(triggered(bool)), editor, SLOT(moveDown(bool)));
    QObject::connect(&actionLeftChannel, SIGNAL(triggered(bool)), editor, SLOT(moveLeft(bool)));
    QObject::connect(&actionRightChannel, SIGNAL(triggered(bool)), editor, SLOT(moveRight(bool)));
    QObject::connect(&actionSwitchChannel, SIGNAL(triggered(bool)), editor, SLOT(switchChannel(bool)));
    QObject::connect(&actionFirstRow, SIGNAL(triggered(bool)), editor, SLOT(gotoFirstRow(bool)));
    QObject::connect(&actionLastRow, SIGNAL(triggered(bool)), editor, SLOT(gotoLastRow(bool)));
    QObject::connect(&actionNextPattern, SIGNAL(triggered(bool)), editor, SLOT(gotoNextPattern(bool)));
    QObject::connect(&actionPreviousPattern, SIGNAL(triggered(bool)), editor, SLOT(gotoPreviousPattern(bool)));

    // Pattern context menu
    QObject::connect(&actionSetStartPattern, SIGNAL(triggered(bool)), this, SLOT(setStartPattern(bool)));
    QObject::connect(&actionRenamePattern, SIGNAL(triggered(bool)), this, SLOT(renamePattern(bool)));
    QObject::connect(&actionSetGoto, SIGNAL(triggered(bool)), this, SLOT(setGoto(bool)));
    QObject::connect(&actionRemoveGoto, SIGNAL(triggered(bool)), this, SLOT(removeGoto(bool)));
    QObject::connect(&actionMovePatternUp, SIGNAL(triggered(bool)), this, SLOT(movePatternUp(bool)));
    QObject::connect(&actionMovePatternDown, SIGNAL(triggered(bool)), this, SLOT(movePatternDown(bool)));
    QObject::connect(&actionInsertPatternBefore, SIGNAL(triggered(bool)), this, SLOT(insertPatternBefore(bool)));
    QObject::connect(&actionInsertPatternAfter, SIGNAL(triggered(bool)), this, SLOT(insertPatternAfter(bool)));
    QObject::connect(&actionRemovePattern, SIGNAL(triggered(bool)), this, SLOT(removePattern(bool)));
    QObject::connect(&actionDuplicatePattern, SIGNAL(triggered(bool)), this, SLOT(duplicatePattern(bool)));

    // Channel context menu
    QObject::connect(&actionSlide, SIGNAL(triggered(bool)), this, SLOT(setSlideValue(bool)));
    QObject::connect(&actionSetFrequency, SIGNAL(triggered(bool)), this, SLOT(setFrequency(bool)));
    QObject::connect(&actionHold, SIGNAL(triggered(bool)), this, SLOT(setHold(bool)));
    QObject::connect(&actionPause, SIGNAL(triggered(bool)), this, SLOT(setPause(bool)));
    QObject::connect(&actionDeleteRow, SIGNAL(triggered(bool)), this, SLOT(deleteRow(bool)));
    QObject::connect(&actionInsertRowBefore, SIGNAL(triggered(bool)), this, SLOT(insertRowBefore(bool)));
    QObject::connect(&actionInsertRowAfter, SIGNAL(triggered(bool)), this, SLOT(insertRowAfter(bool)));
    QObject::connect(&actionMuteChannel, SIGNAL(triggered(bool)), this, SLOT(toggleMute(bool)));

    editor->registerPatternMenu(&patternContextMenu);
    editor->registerChannelMenu(&channelContextMenu);
    timeline->registerPatternMenu(&patternContextMenu);
}

/*************************************************************************/

void TrackTab::updateTrackTab() {
    // Set GUI elements
    QCheckBox *cbGlobal = findChild<QCheckBox *>("checkBoxGlobalTempo");
    cbGlobal->blockSignals(true);
    cbGlobal->setChecked(pTrack->globalSpeed);
    cbGlobal->blockSignals(false);
    QSpinBox *spEven = findChild<QSpinBox *>("spinBoxEvenTempo");
    spEven->blockSignals(true);
    QSpinBox *spOdd = findChild<QSpinBox *>("spinBoxOddTempo");
    spOdd->blockSignals(true);
    if (pTrack->globalSpeed) {
        spEven->setValue(pTrack->evenSpeed);
        spOdd->setValue(pTrack->oddSpeed);
    } else {
        PatternEditor *pe = findChild<PatternEditor *>("trackEditor");
        int editPos = pe->getEditPos();
        if (editPos < pTrack->getChannelNumRows(0)) {
            int patternIndex = pTrack->getPatternIndex(0, editPos);
            spEven->setValue(pTrack->patterns[patternIndex].evenSpeed);
            spOdd->setValue(pTrack->patterns[patternIndex].oddSpeed);
        }
    }
    spEven->blockSignals(false);
    spOdd->blockSignals(false);
    QSpinBox *spRowsPerBeat = findChild<QSpinBox *>("spinBoxRowsPerBeat");
    spRowsPerBeat->blockSignals(true);
    spRowsPerBeat->setValue(pTrack->rowsPerBeat);
    spRowsPerBeat->blockSignals(false);

    // Update individual sub-widgets
    updateTrackStats();
}

/*************************************************************************/

void TrackTab::toggleGlobalTempo(bool toggled) {

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetValueCommand<bool>(pTrack, pTrack->globalSpeed, toggled);

    cmd->setText("Set global speed");

    // no pre step in cmd
    // always post step for status bar update
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd);
}

/*************************************************************************/

void TrackTab::setEvenSpeed(int value) {

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    TrackCommand* cmd;
    if (pTrack->globalSpeed) {
        cmd = new SetValueCommand<int>(pTrack, pTrack->evenSpeed, value);
    } else {
        PatternEditor *pe = findChild<PatternEditor *>("trackEditor");
        int editPos = pe->getEditPos();
        // Only the left channel is used for local tempo
        int patternIndex = pTrack->getPatternIndex(0, editPos);
        cmd = new SetPatternSpeedCommand(pTrack, patternIndex, true, value);
    }

    cmd->setText("Set even speed");

    // no pre step in cmd
    // always post step for status bar update
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd);
}

/*************************************************************************/

void TrackTab::setOddSpeed(int value) {

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    TrackCommand* cmd;
    if (pTrack->globalSpeed) {
        cmd = new SetValueCommand<int>(pTrack, pTrack->oddSpeed, value);
    } else {
        PatternEditor *pe = findChild<PatternEditor *>("trackEditor");
        int editPos = pe->getEditPos();
        int patternIndex = pTrack->getPatternIndex(0, editPos);
        pTrack->patterns[patternIndex].oddSpeed = value;
        cmd = new SetPatternSpeedCommand(pTrack, patternIndex, false, value);
    }

    cmd->setText("Set odd speed");

    // no pre step in cmd
    // always post step for status bar update
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd);
}

/*************************************************************************/

void TrackTab::channelContextEvent(int channel, int noteIndex) {
    contextEventChannel = channel;
    contextEventNoteIndex = noteIndex;
}

/*************************************************************************/

void TrackTab::toggleFollow(bool) {
    QCheckBox *cb = findChild<QCheckBox *>("checkBoxFollow");
    cb->toggle();
}

/*************************************************************************/

void TrackTab::toggleLoop(bool) {
    QCheckBox *cb = findChild<QCheckBox *>("checkBoxLoop");
    cb->toggle();
}

/*************************************************************************/

void TrackTab::setStartPattern(bool) {

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto startPattern = pTrack->getSequenceEntryIndex(contextEventChannel, contextEventNoteIndex);

    auto cmd = new SetStartPatternCommand(pTrack, contextEventChannel, startPattern);

    cmd->setText("Set start pattern");

    // no pre step in cmd
    // always post step for status bar update
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd);
}

/*************************************************************************/

void TrackTab::renamePattern(bool) {
    emit stopTrack(); // do not remove because of modal dialog below 
    RenamePatternDialog dialog(this);
    int entryIndex = pTrack->getSequenceEntryIndex(contextEventChannel, contextEventNoteIndex);
    int patternIndex = pTrack->channelSequences[contextEventChannel].sequence[entryIndex].patternIndex;
    dialog.setPatternName(pTrack->patterns[patternIndex].name);
    if (dialog.exec() != QDialog::Accepted)
        return;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new RenamePatternCommand(pTrack, patternIndex, dialog.getPatternName());

    cmd->setText("Rename pattern");

    // no pre step in cmd
    // always post step for status bar update
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd);
}

/*************************************************************************/

void TrackTab::setGoto(bool) {
    emit stopTrack(); // do not remove because of modal dialog below 
    int maxValue = pTrack->channelSequences[contextEventChannel].sequence.size();
    SetGotoDialog dialog(this);
    dialog.setMaxValue(maxValue);
    int entryIndex = pTrack->getSequenceEntryIndex(contextEventChannel, contextEventNoteIndex);
    Track::SequenceEntry *entry = &(pTrack->channelSequences[contextEventChannel].sequence[entryIndex]);
    dialog.setGotoValue(std::max(1, entry->gotoTarget + 1));
    if (dialog.exec() != QDialog::Accepted)
        return;

    int gotoTarget = dialog.getGotoValue() - 1;
    if (gotoTarget == entry->gotoTarget)
        return;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetGotoCommand(pTrack, contextEventChannel, entryIndex, gotoTarget);

    cmd->setText("Set Goto");

    // no pre step in cmd
    // always post step for status bar update
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd);
}

/*************************************************************************/

void TrackTab::removeGoto(bool) {
    int entryIndex = pTrack->getSequenceEntryIndex(contextEventChannel, contextEventNoteIndex);
    Track::SequenceEntry *entry = &(pTrack->channelSequences[contextEventChannel].sequence[entryIndex]);
    if (entry->gotoTarget == -1)
        return;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetGotoCommand(pTrack, contextEventChannel, entryIndex, -1);

    cmd->setText("Remove Goto");

    // no pre step in cmd
    // always post step for status bar update
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd);
}

/*************************************************************************/

void TrackTab::movePattern(bool isUp, int entryIndex) {

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new MovePatternCommand(pTrack, contextEventChannel, entryIndex, entryIndex + (isUp?-1:1));

    cmd->setText(isUp ? "Move pattern Up" : "Move pattern Down");

    // stop track as a pre step in cmd
    cmd->pre = this->window()->findChild<UndoStep*>("StopTrack");
    // always post step for status bar update
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd);
}

/*************************************************************************/

void TrackTab::movePatternUp(bool) {
    int entryIndex = pTrack->getSequenceEntryIndex(contextEventChannel, contextEventNoteIndex);
    if (entryIndex <= 0)
        return;

    movePattern(true, entryIndex);
}

/*************************************************************************/

void TrackTab::movePatternDown(bool) {
    int entryIndex = pTrack->getSequenceEntryIndex(contextEventChannel, contextEventNoteIndex);
    if (entryIndex == pTrack->channelSequences[contextEventChannel].sequence.size() - 1)
        return;

    movePattern(false, entryIndex);
}

/*************************************************************************/

void TrackTab::insertPatternBefore(bool) {
    insertPattern(true);
}

/*************************************************************************/

void TrackTab::insertPatternAfter(bool) {
    insertPattern(false);
}

/*************************************************************************/

void TrackTab::insertPattern(bool doBefore) {

    emit stopTrack(); // do not remove because of modal dialog below
    TrackCommand* macro = nullptr;
    QString patternName;
    int patternIndex = choosePatternToInsert(doBefore, macro, patternName);
    if (patternIndex == -1) {
        return;
    }

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new InsertPatternCommand(pTrack, patternIndex, contextEventChannel,
        pTrack->getSequenceEntryIndex(contextEventChannel, contextEventNoteIndex) + (doBefore ? 0 : 1),
        macro);

    auto upper_cmd = macro ? macro : cmd;

    QString cmdName = "Insert pattern \"" + patternName + (doBefore?"\" (before)": "\" (after)");
    upper_cmd->setText(cmdName);

    // stop track as a pre step in cmd, update tab update as a post step
    upper_cmd->pre = this->window()->findChild<UndoStep*>("StopTrack");
    upper_cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    // hold gui stuffs in the uppest command:
    upper_cmd->ci.tab = MainWindow::iTabTrack;

    PatternEditor *pe = findChild<PatternEditor *>("trackEditor");

    upper_cmd->ci.editPosFrom = pe->getEditPos();

    undoStack->push(upper_cmd); // pre, post and redo methods are called here
}

/*************************************************************************/

void TrackTab::removePattern(bool) {
    emit stopTrack(); // do not remove because of modal dialog(s) below
    if (pTrack->channelSequences[contextEventChannel].sequence.size() == 1) {
        MainWindow::displayMessage("A channel must contain at least one pattern!");
        return;
    }

    int entryIndex = pTrack->getSequenceEntryIndex(contextEventChannel, contextEventNoteIndex);
    int patternIndex = pTrack->channelSequences[contextEventChannel].sequence[entryIndex].patternIndex;
    
    // Check if pattern is no longer used anywhere
    bool wasLast = true;
    int found = 0;
    for (int channel = 0; channel < 2 && wasLast; ++channel) {
        for (int i = 0; i < pTrack->channelSequences[channel].sequence.size(); ++i) {
            if (pTrack->channelSequences[channel].sequence[i].patternIndex == patternIndex) {
                if (++found > 1) {
                    wasLast = false;
                    break;
                }
            }
        }
    }
    bool deletePattern = false;
    if (wasLast) {
        QMessageBox msgBox(QMessageBox::NoIcon,
                           "Delete Pattern?",
                           "This pattern is no longer used in the track. Do you want to delete it?",
                           QMessageBox::Yes | QMessageBox::No, this,
                           Qt::FramelessWindowHint);
        deletePattern = (msgBox.exec() == QMessageBox::Yes);
    }

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new RemovePatternCommand(pTrack, patternIndex, contextEventChannel, entryIndex, deletePattern);

    cmd->setText(deletePattern ? "Remove and Delete pattern" : "Remove pattern");

    // stop track as a pre step in cmd, update tab update as a post step
    cmd->pre = this->window()->findChild<UndoStep*>("StopTrack");
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    // hold gui stuffs in the command:
    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd);

    PatternEditor *pe = findChild<PatternEditor *>("trackEditor");

    cmd->ci.editPosFrom = pe->getEditPos();

    emit validateEditPos();

    cmd->ci.selectedChannel = contextEventChannel;
    cmd->ci.editPosTo = pe->getEditPos();
}

/*************************************************************************/

void TrackTab::duplicatePattern(bool) {
    emit stopTrack(); // do not remove because of modal dialog below
    RenamePatternDialog dialog(this);
    int entryIndex = pTrack->getSequenceEntryIndex(contextEventChannel, contextEventNoteIndex);
    int patternIndex = pTrack->channelSequences[contextEventChannel].sequence[entryIndex].patternIndex;
    dialog.setPatternName(pTrack->patterns[patternIndex].name);
    if (dialog.exec() != QDialog::Accepted)
        return;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new DuplicatePatternCommand(pTrack, patternIndex, contextEventChannel, entryIndex,
        dialog.getPatternName());

    cmd->setText("Duplicate pattern");

    // stop track as a pre step in cmd, update tab update as a post step
    cmd->pre = this->window()->findChild<UndoStep*>("StopTrack");
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    // hold gui stuffs in the command:
    cmd->ci.tab = MainWindow::iTabTrack;

    PatternEditor *pe = findChild<PatternEditor *>("trackEditor");

    cmd->ci.editPosFrom = pe->getEditPos();

    undoStack->push(cmd);
}

/*************************************************************************/

void TrackTab::setSlideValue(bool) {
    emit stopTrack();  // do not remove because of modal dialog below
    if (!pTrack->checkSlideValidity(contextEventChannel, contextEventNoteIndex)) {
        MainWindow::displayMessage("A SLIDE can only follow a melodic instrument or another slide!");
        return;
    }

    SetSlideDialog dialog(this);
    // If selected row already contains a SLIDE, pre-set dialog to that value
    Track::Note *selectedNote = pTrack->getNote(contextEventChannel, contextEventNoteIndex);
    if (selectedNote->type == Track::Note::instrumentType::Slide) {
        dialog.setSlideValue(selectedNote->value);
    }
    if (dialog.exec() != QDialog::Accepted)
        return;
    
    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    int patternIndex = pTrack->getPatternIndex(contextEventChannel, contextEventNoteIndex);
    int noteIndex = pTrack->getNoteIndexInPattern(contextEventChannel, contextEventNoteIndex);

    auto cmd = new SetSlideValueCommand(pTrack, patternIndex, noteIndex, dialog.getSlideValue());

    cmd->setText("Set Slide Value");

    // stop track as a pre step in cmd, update tab update as a post step
    cmd->pre = this->window()->findChild<UndoStep*>("StopTrack");
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    PatternEditor *pe = findChild<PatternEditor *>("trackEditor");

    // hold gui stuffs in cmd:
    cmd->ci.tab = MainWindow::iTabTrack;
    cmd->ci.selectedChannel = contextEventChannel;
    cmd->ci.editPosFrom = pe->getEditPos();
    cmd->ci.editPosTo = pe->getEditPos() + 1;

    undoStack->push(cmd); // post and redo methods are called here
}

/*************************************************************************/

void TrackTab::setFrequency(bool) {
    emit stopTrack(); // do not remove because of modal dialog below
    Track::Note *selectedNote = pTrack->getNote(contextEventChannel, contextEventNoteIndex);
    if (selectedNote->type != Track::Note::instrumentType::Instrument) {
        MainWindow::displayMessage("Only a melodic instrument can have a frequency value!");
        return;
    }
    SetFrequencyDialog dialog(this);
    dialog.setFrequencyValue(selectedNote->value);
    Track::Instrument *ins = &(pTrack->instruments[selectedNote->instrumentNumber]);
    int maxFreq = ins->baseDistortion == TiaSound::Distortion::PURE_COMBINED ? 63 : 31;
    dialog.setMaxFrequencyValue(maxFreq);
    if (dialog.exec() != QDialog::Accepted)
        return;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    int patternIndex = pTrack->getPatternIndex(contextEventChannel, contextEventNoteIndex);
    int noteIndex = pTrack->getNoteIndexInPattern(contextEventChannel, contextEventNoteIndex);

    auto cmd = new SetRowToInstrumentCommand(pTrack, patternIndex, noteIndex, selectedNote->instrumentNumber, dialog.getFrequencyValue());

    cmd->setText("Set Frequency");

    PatternEditor *pe = findChild<PatternEditor *>("trackEditor");

    // hold gui stuffs in cmd:
    cmd->ci.tab = MainWindow::iTabTrack;
    cmd->ci.selectedChannel = contextEventChannel;
    cmd->ci.editPosFrom = pe->getEditPos();
    cmd->ci.editPosTo = pe->getEditPos() + 1;

    undoStack->push(cmd); // post and redo methods are called here
}

/*************************************************************************/

void TrackTab::setHold(bool) {
    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    int patternIndex = pTrack->getPatternIndex(contextEventChannel, contextEventNoteIndex);
    int noteIndex = pTrack->getNoteIndexInPattern(contextEventChannel, contextEventNoteIndex);

    auto cmd = new SetHoldCommand(pTrack, patternIndex, noteIndex);

    cmd->setText("Set Hold");

    // stop track as a pre step in cmd, update tab update as a post step
    cmd->pre = this->window()->findChild<UndoStep*>("StopTrack");
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    PatternEditor *pe = findChild<PatternEditor *>("trackEditor");

    // hold gui stuffs in cmd:
    cmd->ci.tab = MainWindow::iTabTrack;
    cmd->ci.selectedChannel = contextEventChannel;
    cmd->ci.editPosFrom = pe->getEditPos();
    cmd->ci.editPosTo = pe->getEditPos() + 1;

    undoStack->push(cmd); // post and redo methods are called here
}

/*************************************************************************/

void TrackTab::setPause(bool) {
    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    int patternIndex = pTrack->getPatternIndex(contextEventChannel, contextEventNoteIndex);
    int noteIndex = pTrack->getNoteIndexInPattern(contextEventChannel, contextEventNoteIndex);

    auto cmd = new SetPauseCommand(pTrack, patternIndex, noteIndex);

    cmd->setText("Set Pause");

    // stop track as a pre step in cmd, update tab update as a post step
    cmd->pre = this->window()->findChild<UndoStep*>("StopTrack");
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    PatternEditor *pe = findChild<PatternEditor *>("trackEditor");

    // hold gui stuffs in cmd:
    cmd->ci.tab = MainWindow::iTabTrack;
    cmd->ci.selectedChannel = contextEventChannel;
    cmd->ci.editPosFrom = pe->getEditPos();
    cmd->ci.editPosTo = pe->getEditPos() + 1;

    undoStack->push(cmd); // post and redo methods are called here
}

/*************************************************************************/

void TrackTab::deleteRow(bool) {
    emit stopTrack(); // do not remove because of modal dialog below
    int patternIndex = pTrack->getPatternIndex(contextEventChannel, contextEventNoteIndex);
    if (pTrack->patterns[patternIndex].notes.size() == Track::Pattern::minSize) {
        MainWindow::displayMessage("Pattern is already at minimum size!");
        return;
    }

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    int noteIndex = pTrack->getNoteIndexInPattern(contextEventChannel, contextEventNoteIndex);

    auto cmd = new DeleteRowCommand(pTrack, patternIndex, noteIndex);

    cmd->setText("Delete Row");

    // stop track as a pre step in cmd, update tab update as a post step
    cmd->pre = this->window()->findChild<UndoStep*>("StopTrack");
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd); // post and redo methods are called here

    PatternEditor *pe = findChild<PatternEditor *>("trackEditor");

    // hold gui stuffs in cmd:
    cmd->ci.editPosFrom = pe->getEditPos();

    emit validateEditPos();

    cmd->ci.selectedChannel = contextEventChannel;
    cmd->ci.editPosTo = pe->getEditPos();
}

/*************************************************************************/

void TrackTab::insertRowBefore(bool) {
    emit stopTrack(); // do not remove because of modal dialog below
    int patternIndex = pTrack->getPatternIndex(contextEventChannel, contextEventNoteIndex);
    if (pTrack->patterns[patternIndex].notes.size() == Track::Pattern::maxSize) {
        MainWindow::displayMessage("Pattern is already at maximum size!");
        return;
    }

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    int noteIndex = pTrack->getNoteIndexInPattern(contextEventChannel, contextEventNoteIndex);

    auto cmd = new InsertRowCommand(pTrack, patternIndex, noteIndex);

    cmd->setText("Insert Row (before)");

    // stop track as a pre step in cmd, update tab update as a post step
    cmd->pre = this->window()->findChild<UndoStep*>("StopTrack");
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd); // post and redo methods are called here
}

/*************************************************************************/

void TrackTab::insertRowAfter(bool) {
    emit stopTrack(); // do not remove because of modal dialog below
    int patternIndex = pTrack->getPatternIndex(contextEventChannel, contextEventNoteIndex);
    if (pTrack->patterns[patternIndex].notes.size() == Track::Pattern::maxSize) {
        MainWindow::displayMessage("Pattern is already at maximum size!");
        return;
    }

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    int noteIndex = pTrack->getNoteIndexInPattern(contextEventChannel, contextEventNoteIndex)+1;

    auto cmd = new InsertRowCommand(pTrack, patternIndex, noteIndex);

    cmd->setText("Insert Row (after)");

    // stop track as a pre step in cmd, update tab update as a post step
    cmd->pre = this->window()->findChild<UndoStep*>("StopTrack");
    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabTrack;

    undoStack->push(cmd); // post and redo methods are called here
}

/*************************************************************************/

void TrackTab::toggleMute(bool) {
    pPlayer->channelMuted[contextEventChannel] = !pPlayer->channelMuted[contextEventChannel];
}

/*************************************************************************/

void TrackTab::invalidNoteFound(int, int, int, QString reason) {
    MainWindow::displayMessage(reason);
}

/*************************************************************************/

void TrackTab::updateTrackStats() {
    // Patterns
    QLabel *statsLabel = findChild<QLabel *>("labelPatternsUsed");
    int sequenceLength = pTrack->channelSequences[0].sequence.size() + pTrack->channelSequences[1].sequence.size();
    int distinct = pTrack->patterns.size();
    statsLabel->setText("Patterns used: " + QString::number(sequenceLength) + " of 255 ("
                        + QString::number(distinct) + " distinct)");

    // Start patterns
    QLabel *startLabel = findChild<QLabel *>("labelStartPatterns");
    startLabel->setText("Start patterns: " + QString::number(pTrack->startPatterns[0] + 1)
                        + "/" + QString::number(pTrack->startPatterns[1] + 1));

    // Time
    int numRows = pTrack->getTrackNumRows();
    long numOddTicks = int((numRows + 1)/2)*pTrack->oddSpeed;
    long numEvenTicks = int(numRows/2)*pTrack->evenSpeed;
    long numTicks = numOddTicks + numEvenTicks;
    int ticksPerSecond = pTrack->tvMode == TiaSound::TvStandard::PAL ? 50 : 60;
    int minutes = numTicks/(ticksPerSecond*60);
    int seconds = (numTicks%(ticksPerSecond*60))/ticksPerSecond;
    QString timeText = QString::number(minutes);
    if (seconds < 10) {
        timeText.append("m 0");
    } else {
        timeText.append("m ");
    }
    timeText.append(QString::number(seconds) + "s");
    QLabel *timeLabel = findChild<QLabel *>("labelTotalLength");
    timeLabel->setText("Total length: " + timeText);
    // BPM
    QLabel *bpmLabel = findChild<QLabel *>("labelBPM");
    if (pTrack->globalSpeed) {
        int numTicksSum = pTrack->oddSpeed + pTrack->evenSpeed;
        double bpm = (60.0*ticksPerSecond)/((numTicksSum/2.0)*pTrack->rowsPerBeat);
        bpmLabel->setText("(~" + QString::number(bpm, 'f', 2) + " bpm)");
    } else {
        bpmLabel->setText("n/a (local tempo)");
    }
}

/*************************************************************************/

int TrackTab::choosePatternToInsert(bool doBefore, TrackCommand*& macro, QString& patternName) {
    InsertPatternDialog dialog(this);
    dialog.prepare(pTrack);
    if (dialog.exec() == QDialog::Accepted) {
        int result = dialog.getSelectedPattern();
        // Check for "create new pattern"
        if (result == pTrack->patterns.size()) {
            // Calc index of first note of new pattern, for "align" button
            int entryIndex = pTrack->getSequenceEntryIndex(contextEventChannel, contextEventNoteIndex);
            int patternIndex = pTrack->channelSequences[contextEventChannel].sequence[entryIndex].patternIndex;
            int newRow = pTrack->channelSequences[contextEventChannel].sequence[entryIndex].firstNoteNumber;
            if (!doBefore) {
                newRow += pTrack->patterns[patternIndex].notes.size();
            }
            CreatePatternDialog newDialog(this);
            newDialog.prepare(pTrack, lastNewPatternLength, contextEventChannel, newRow);
            if (newDialog.exec() == QDialog::Accepted) {
                QString newName = newDialog.getName();
                int newLength = newDialog.getLength();
                Track::Pattern newPattern(newName);
                QSpinBox *spEven = findChild<QSpinBox *>("spinBoxEvenTempo");
                // Get even/odd speeds from GUI
                newPattern.evenSpeed = spEven->value();
                QSpinBox *spOdd = findChild<QSpinBox *>("spinBoxOddTempo");
                newPattern.oddSpeed = spOdd->value();
                // Create notes
                for (int i = 0; i < newLength; ++i) {
                    Track::Note newNote(Track::Note::instrumentType::Hold, 0, 0);
                    newPattern.notes.append(newNote);
                }
                // Add new pattern
                macro = new TrackCommand(pTrack, "");
                new CreatePatternCommand(pTrack, newPattern, macro);
                patternName = newPattern.name;
                lastNewPatternLength = newLength;
            } else {
                return -1;
            }
        }
        else {
            patternName = pTrack->patterns[result].name;
        }
        return result;
    } else {
        return -1;
    }
}

/*************************************************************************/

void TrackTab::addShortcut(QAction *action, QString actionName) {
    if (MainWindow::keymap.contains(actionName)) {
        QString shortcut = MainWindow::keymap[actionName].toString();
        action->setShortcut(QKeySequence(shortcut));
        addAction(action);
    }
}
