/* TIATracker, (c) 2016 Andre "Kylearan" Wichmann.
 * Website: https://bitbucket.org/kylearan/tiatracker
 * Email: andre.wichmann@gmx.de
 * See the file "license.txt" for information on usage and redistribution
 * of this file.
 */

#include "waveformshaper.h"
#include <QPainter>
#include "mainwindow.h"
#include "percussiontab.h"
#include <cassert>
#include <QMouseEvent>
#include "tiasound/tiasound.h"
#include <QWheelEvent>
#include "instrumentstab.h"


WaveformShaper::WaveformShaper(QWidget *parent) : QWidget(parent)
{
    valueFont.setPixelSize(valueFontSize);
    QFontMetrics fontMetrics(valueFont);
    valueFontHeight = fontMetrics.height();
    valueAreaHeight = valueFontHeight + 2*valueAreaMargin;
    setFixedWidth(calcWidth());
    setFixedHeight(valueAreaHeight + legendCellSize);

    // Create context menu
    foreach (TiaSound::Distortion dist, PercussionTab::availableWaveforms) {
        contextMenu.addAction(TiaSound::getDistortionName(dist));
    }
    QObject::connect(&contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(setWaveform(QAction*)));
}

/*************************************************************************/

void WaveformShaper::registerPercussion(Track::Percussion *newPercussion) {
    pPercussion = newPercussion;

}

/*************************************************************************/

void WaveformShaper::updateSize() {
    setFixedWidth(calcWidth());
}

/*************************************************************************/

void WaveformShaper::setValues(QList<TiaSound::Distortion> newValues) {
    values = newValues;
    updateSize();
}

/*************************************************************************/

void WaveformShaper::setWaveform(QAction *action) {
    // Get distortion of selected waveform and set pen to it
    foreach (TiaSound::Distortion dist, PercussionTab::availableWaveforms) {
        if (TiaSound::getDistortionName(dist) == action->text()) {
            distortionPen = dist;
            break;
        }
    }
    // Set distortion column
    values[waveformColumn] = distortionPen;
    update();
}

/*************************************************************************/

void WaveformShaper::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    const int envelopeLength = pPercussion->getEnvelopeLength();

    // Left side
    painter.fillRect(0, 0, legendCellSize, valueAreaHeight+legendCellSize, MainWindow::lightHighlighted);
    // Bottom
    painter.fillRect(legendCellSize, valueAreaHeight, envelopeLength*cellWidth, legendCellSize, MainWindow::lightHighlighted);
    // Value area
    painter.fillRect(legendCellSize, 0, envelopeLength*cellWidth, valueAreaHeight, MainWindow::dark);
    // Frame numbers
    painter.setFont(valueFont);
    painter.setPen(MainWindow::contentDarker);
    for (int frame = 0; frame < envelopeLength; ++frame) {
        int xPos = legendCellSize + frame*cellWidth;
        painter.drawText(xPos, valueAreaHeight, cellWidth, legendCellSize, Qt::AlignCenter, QString::number(frame + 1));
    }
    // Values
    painter.setPen(MainWindow::contentLight);
    for (int iValue = 0; iValue < envelopeLength; ++iValue) {
        TiaSound::Distortion value = values[iValue];
        int intValue = TiaSound::getDistortionInt(value);
        painter.drawText(legendCellSize + iValue*cellWidth, valueAreaMargin, cellWidth, valueFontHeight, Qt::AlignCenter, QString::number(intValue));
    }

}

/*************************************************************************/

void WaveformShaper::contextMenuEvent(QContextMenuEvent *event) {
    if (event->x() >= legendCellSize && event->y() < valueAreaHeight) {
        waveformColumn = (event->x() - legendCellSize)/cellWidth;
        contextMenu.exec(event->globalPos());
    }
}

/*************************************************************************/

void WaveformShaper::processMouseEvent(int x, int y) {
    int iValue = int(x/cellWidth);  //this is to make sure mouse is not out of bounds of the waveform box
    if (iValue >=0 && iValue <= pPercussion->getEnvelopeLength()) {
        if (x >= legendCellSize && y < valueAreaHeight) {
           int column = (x - legendCellSize)/cellWidth;
           values[column] = distortionPen;
           update();
           emit valuesChanged(values);
        }
    }

}

void WaveformShaper::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isMouseDragging = true;
        processMouseEvent(event->x(), event->y());
    }
}
//added these events from envelopeshaper to handle dragging
void WaveformShaper::mouseReleaseEvent(QMouseEvent *) {
    isMouseDragging = false;
}

void WaveformShaper::mouseMoveEvent(QMouseEvent *event) {
    if (isMouseDragging) {
        processMouseEvent(event->x(), event->y());
    }
}

/*************************************************************************/

void WaveformShaper::wheelEvent(QWheelEvent *event) {
    if (event->position().x() >= legendCellSize && event->position().y() < valueAreaHeight) {
        int column = (event->position().x() - legendCellSize)/cellWidth;
        int delta = event->angleDelta().y()/100;
        // Get index of current waveform
        TiaSound::Distortion oldDist = values[column];
        int oldIndex = PercussionTab::availableWaveforms.indexOf(oldDist);
        int newIndex = oldIndex + delta;
        newIndex = std::max(newIndex, 0);
        newIndex = std::min(newIndex, (int)(PercussionTab::availableWaveforms.size() - 1));
        values[column] = PercussionTab::availableWaveforms[newIndex];
        update();
        emit valuesChanged(values);
    }
}

/*************************************************************************/

int WaveformShaper::calcWidth() {
    int envelopeLength = 21;
    // During init, no percussion is registered yet
    if (pPercussion != nullptr) {
        envelopeLength = pPercussion->getEnvelopeLength();
    }
    int width = legendCellSize;
    if (!values.empty()) {
        width += envelopeLength*cellWidth;
    }
    return width;
}

/*************************************************************************/

