#include "patterneditor.h"
#include <QFontMetrics>
#include <QPainter>
#include "mainwindow.h"
#include "track/pattern.h"
#include "track/sequence.h"
#include "track/sequenceentry.h"
#include "track/note.h"
#include "tiasound/pitchguidefactory.h"
#include "tiasound/pitchguide.h"
#include "tiasound/instrumentpitchguide.h"
#include "tiasound/tiasound.h"
#include <QWheelEvent>


PatternEditor::PatternEditor(QWidget *parent) : QWidget(parent)
{
    legendFont.setPixelSize(legendFontSize);
    QFontMetrics legendFontMetrics(legendFont);
    legendFontHeight = legendFontMetrics.height();
    timeAreaWidth = legendFontMetrics.width("000:00");

    noteFont.setPixelSize(noteFontSize);
    QFontMetrics noteFontMetrics(noteFont);
    noteFontHeight = noteFontMetrics.height();
    noteAreaWidth = noteFontMetrics.width("000: C#4 I7 31")
            + 2*noteMargin;

    widgetWidth = 2*patternNameWidth
            + 2*noteAreaWidth
            + timeAreaWidth;
    setFixedWidth(widgetWidth);
}

/*************************************************************************/

void PatternEditor::registerTrack(Track::Track *newTrack) {
    pTrack = newTrack;
}

/*************************************************************************/

void PatternEditor::registerPitchGuide(TiaSound::PitchGuide *newGuide) {
    pPitchGuide = newGuide;
}

/*************************************************************************/

void PatternEditor::registerPatternMenu(QMenu *newPatternMenu) {
    pPatternMenu = newPatternMenu;
}

/*************************************************************************/

void PatternEditor::registerChannelMenu(QMenu *newChannelMenu) {
    pChannelMenu = newChannelMenu;
}

/*************************************************************************/

void PatternEditor::setEditPos(int newPos) {
    editPos = newPos;
    if (editPos < 0) {
        editPos = 0;
    }
    if (editPos >= pTrack->getTrackNumRows()) {
        editPos = pTrack->getTrackNumRows() - 1;
    }
    emit editPosChanged(editPos);
    update();
}

/*************************************************************************/

void PatternEditor::setRowsPerBeat(int value) {
    pTrack->rowsPerBeat = value;
    update();
}

/*************************************************************************/

int PatternEditor::getEditPos() {
    return editPos;
}

/*************************************************************************/

QSize PatternEditor::sizeHint() const {
    return QSize(widgetWidth, minHeight);
}

/*************************************************************************/

QString PatternEditor::constructRowString(int curPatternNoteIndex, Track::Pattern *curPattern) {
    QString rowText = QString::number(curPatternNoteIndex + 1);
    if (curPatternNoteIndex + 1 < 10) {
        rowText.prepend("  ");
    } else if (curPatternNoteIndex < 100) {
        rowText.prepend(" ");
    }
    switch (curPattern->notes[curPatternNoteIndex].type) {
    case Track::Note::instrumentType::Hold:
        rowText.append(":    |");
        break;
    case Track::Note::instrumentType::Slide: {
        int frequency = curPattern->notes[curPatternNoteIndex].value;
        rowText.append(":   ");
        rowText.append("  SL");
        // Frequency change
        if (frequency < 0) {
            rowText.append(" ");
        } else {
            rowText.append(" +");
        }
        rowText.append(QString::number(frequency));
        break;
    }
    case Track::Note::instrumentType::Pause:
        rowText.append(":   ---");
        break;
    case Track::Note::instrumentType::Percussion: {
        int percNum = curPattern->notes[curPatternNoteIndex].instrumentNumber + 1;
        if (percNum < 10) {
            rowText.append(":   P ");
        } else {
            rowText.append(":   P");
        }
        rowText.append(QString::number(percNum));
        break;
    }
    case Track::Note::instrumentType::Instrument: {
        int insNum = curPattern->notes[curPatternNoteIndex].instrumentNumber + 1;
        // Pitch
        int frequency = curPattern->notes[curPatternNoteIndex].value;
        TiaSound::Distortion dist = pTrack->instruments[insNum].baseDistortion;
        TiaSound::InstrumentPitchGuide *pIPG = &(pPitchGuide->instrumentGuides[dist]);
        TiaSound::Note note = pIPG->getNote(frequency);
        if (note == TiaSound::Note::NotANote) {
            rowText.append(": ???");
        } else {
            rowText.append(": ");
            rowText.append(TiaSound::getNoteNameWithOctaveFixedWidth(note));
        }
        // Instrument number
        rowText.append(" I");
        rowText.append(QString::number(insNum));
        // Frequency
        if (frequency < 10) {
            rowText.append("  ");
        } else {
            rowText.append(" ");
        }
        rowText.append(QString::number(frequency));
        break;
    }
    default:
        rowText.append(": ??? ");
        break;
    }

    return rowText;
}

void PatternEditor::drawPatternNameAndSeparator(int yPos, int nameXPos, int curPatternNoteIndex, int channel, int xPos, int curEntryIndex, QPainter *painter, Track::Pattern *curPattern)
{
    if (curPatternNoteIndex == 0) {
        painter->fillRect(xPos - noteMargin, yPos, noteAreaWidth, 1, MainWindow::contentDarker);
        painter->setFont(legendFont);
        painter->setPen(MainWindow::contentDarker);
        int alignment = channel == 0 ? Qt::AlignRight : Qt::AlignLeft;
        QString patternName = QString::number(curEntryIndex + 1);
        patternName.append(": ");
        patternName.append(curPattern->name);
        if (curEntryIndex == pTrack->startPatterns[channel]) {
            painter->setPen(MainWindow::green);

        } else {
            painter->setPen(MainWindow::blue);
        }
        painter->drawText(nameXPos, yPos, patternNameWidth - 2*patternNameMargin, legendFontHeight, alignment, patternName);
    }
}

void PatternEditor::drawGoto(int channel, int yPos, Track::Pattern *curPattern, Track::SequenceEntry *curEntry, QPainter *painter, int nameXPos, int curPatternNoteIndex)
{
    if (curPatternNoteIndex == curPattern->notes.size() - 1
            && curEntry->gotoTarget != -1) {
        int alignment = channel == 0 ? Qt::AlignRight : Qt::AlignLeft;
        painter->setFont(legendFont);
        painter->setPen(MainWindow::blue);
        painter->drawText(nameXPos, yPos, patternNameWidth - 2*patternNameMargin, legendFontHeight, alignment,
                          "GOTO " + QString::number(curEntry->gotoTarget + 1));
    }
}

void PatternEditor::drawTimestamp(int row, QPainter *painter, int yPos, int channel)
{
    int ticksPerSecond = pTrack->getTvMode() == TiaSound::TvStandard::PAL ? 50 : 60;
    long numOddTicks = int((row + 1)/2)*pTrack->oddSpeed;
    long numEvenTicks = int(row/2)*pTrack->evenSpeed;
    long numTick = numOddTicks + numEvenTicks;
    int curTicks = row%2 == 0 ? pTrack->evenSpeed : pTrack->oddSpeed;
    if (channel == 0 && numTick%ticksPerSecond < curTicks) {
        int minute = numTick/(ticksPerSecond*60);
        int second = (numTick%(ticksPerSecond*60))/ticksPerSecond;
        QString timestampText = QString::number(minute);
        if (second < 10) {
            timestampText.append(":0");
        } else {
            timestampText.append(":");
        }
        timestampText.append(QString::number(second));
        painter->setFont(legendFont);
        painter->setPen(MainWindow::contentDarker);
        painter->drawText(patternNameWidth + noteAreaWidth, yPos, timeAreaWidth, legendFontHeight, Qt::AlignHCenter, timestampText);
    }
}

void PatternEditor::paintChannel(QPainter *painter, int channel, int xPos, int yOffset, int numRows, int nameXPos) {
    // Calc first note/pattern
    int firstNoteIndex = max(0, editPos - numRows/2);
    // Don't do anything if we are behind the last note
    int channelSize = pTrack->getChannelNumRows(channel);
    if (firstNoteIndex >= channelSize) {
        return;
    }
    // Get pointers to first note to paint
    int curEntryIndex = 0;
    Track::SequenceEntry *curEntry = &(pTrack->channelSequences[channel].sequence[0]);
    Track::Pattern *curPattern = &(pTrack->patterns[curEntry->patternIndex]);
    while (firstNoteIndex >= curEntry->firstNoteNumber + curPattern->notes.size()) {
        curEntryIndex++;
        curEntry = &(pTrack->channelSequences[channel].sequence[curEntryIndex]);
        curPattern = &(pTrack->patterns[curEntry->patternIndex]);
    }
    int curPatternNoteIndex = firstNoteIndex - curEntry->firstNoteNumber;
    // Draw rows
    for (int row = firstNoteIndex; row <= editPos + numRows/2; ++row) {
        int yPos = yOffset + noteFontHeight*(row - (editPos - numRows/2));
        // First row in beat?
        if (row%(pTrack->rowsPerBeat) == 0 && (channel != selectedChannel || row != editPos)) {
            painter->fillRect(xPos - noteMargin, yPos, noteAreaWidth, noteFontHeight, MainWindow::darkHighlighted);
        }
        QString rowText = constructRowString(curPatternNoteIndex, curPattern);
        painter->setFont(noteFont);
        painter->setPen(MainWindow::blue);
        painter->drawText(xPos, yPos, noteAreaWidth - 2*noteMargin, noteFontHeight, Qt::AlignLeft, rowText);

        drawPatternNameAndSeparator(yPos, nameXPos, curPatternNoteIndex, channel, xPos, curEntryIndex, painter, curPattern);
        drawGoto(channel, yPos, curPattern, curEntry, painter, nameXPos, curPatternNoteIndex);
        drawTimestamp(row, painter, yPos, channel);

        // Advance note
        if (!pTrack->getNextNote(channel, &curEntryIndex, &curPatternNoteIndex)) {
            // End of track reached: Stop drawing
            break;
        }
        curEntry = &(pTrack->channelSequences[channel].sequence[curEntryIndex]);
        curPattern = &(pTrack->patterns[curEntry->patternIndex]);
    }
}

void PatternEditor::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    // Pattern name areas
    painter.fillRect(0, 0, patternNameWidth, height(), MainWindow::lightHighlighted);
    painter.fillRect(widgetWidth - patternNameWidth, 0, patternNameWidth, height(), MainWindow::lightHighlighted);
    // Note areas
    painter.fillRect(patternNameWidth, 0, noteAreaWidth, height(), MainWindow::dark);
    painter.fillRect(patternNameWidth + noteAreaWidth + timeAreaWidth, 0, noteAreaWidth, height(), MainWindow::dark);
    // Time area
    painter.fillRect(patternNameWidth + noteAreaWidth, 0, timeAreaWidth, height(), MainWindow::lightHighlighted);
    // Current highlights
    int highlightY = height()/2 - noteFontHeight/2;
    int highlightX = patternNameWidth + selectedChannel*(noteAreaWidth + timeAreaWidth);
    painter.fillRect(highlightX, highlightY, noteAreaWidth, noteFontHeight, MainWindow::light);

    // Calc number of visible rows
    int numRows = height()/noteFontHeight;
    if (numRows%2 == 0) {
        numRows--;
    }
    int topMargin = (height() - numRows*noteFontHeight)/2;

    // Paint channels
    paintChannel(&painter, 0, patternNameWidth + noteMargin, topMargin, numRows, patternNameMargin);
    paintChannel(&painter, 1, patternNameWidth + noteAreaWidth + timeAreaWidth + noteMargin, topMargin, numRows, width() - patternNameWidth + patternNameMargin);

}

/*************************************************************************/

void PatternEditor::wheelEvent(QWheelEvent *event) {
    int newPos = editPos - event->delta()/100;
    setEditPos(newPos);
}

/*************************************************************************/

void PatternEditor::mousePressEvent(QMouseEvent *event) {
    // TODO
}

/*************************************************************************/

void PatternEditor::contextMenuEvent(QContextMenuEvent *event) {
    if (event->x() < patternNameWidth) {
        pPatternMenu->exec(event->globalPos());
    } else if (event->x() >= patternNameWidth && event->x() < patternNameWidth + noteAreaWidth) {
        pChannelMenu->exec(event->globalPos());
    } else if (event->x() >= patternNameWidth + noteAreaWidth + timeAreaWidth
               && event->x() < patternNameWidth + noteAreaWidth + timeAreaWidth + noteAreaWidth) {
        pChannelMenu->exec(event->globalPos());
    } else if (event->x() >= patternNameWidth + noteAreaWidth + timeAreaWidth + noteAreaWidth
               && event->x() < patternNameWidth + noteAreaWidth + timeAreaWidth + noteAreaWidth + patternNameWidth){
        pPatternMenu->exec(event->globalPos());
    }
}
