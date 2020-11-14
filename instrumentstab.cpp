/* TIATracker, (c) 2016 Andre "Kylearan" Wichmann.
 * Website: https://bitbucket.org/kylearan/tiatracker
 * Email: andre.wichmann@gmx.de
 * See the file "license.txt" for information on usage and redistribution
 * of this file.
 */

#include "instrumentstab.h"
#include <QComboBox>
#include <QLineEdit>
#include <cassert>
#include <QLabel>
#include <QSpinBox>
#include "envelopeshaper.h"
#include <QMessageBox>
#include <iostream>
#include <QFileDialog>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "mainwindow.h"


const QList<TiaSound::Distortion> InstrumentsTab::availableWaveforms{
    TiaSound::Distortion::BUZZY,
    TiaSound::Distortion::BUZZY_RUMBLE,
    TiaSound::Distortion::FLANGY_WAVERING,
    TiaSound::Distortion::PURE_HIGH,
    TiaSound::Distortion::PURE_BUZZY,
    TiaSound::Distortion::REEDY_RUMBLE,
    TiaSound::Distortion::WHITE_NOISE,
    TiaSound::Distortion::PURE_LOW,
    TiaSound::Distortion::ELECTRONIC_RUMBLE,
    TiaSound::Distortion::ELECTRONIC_SQUEAL,
    TiaSound::Distortion::PURE_COMBINED
};

/*************************************************************************/

InstrumentsTab::InstrumentsTab(QWidget *parent) : QWidget(parent)
{

}

/*************************************************************************/

void InstrumentsTab::registerTrack(Track::Track *newTrack) {
    pTrack = newTrack;
}

/*************************************************************************/

void InstrumentsTab::initInstrumentsTab() {
    // Instrument names
    QComboBox *cbInstruments = findChild<QComboBox *>("comboBoxInstruments");
    cbInstruments->lineEdit()->setMaxLength(maxInstrumentNameLength);
    foreach(Track::Instrument ins, pTrack->instruments) {
        cbInstruments->addItem(ins.name);
    }
    // Instrument waveforms
    QComboBox *cbWaveforms = findChild<QComboBox *>("comboBoxWaveforms");
    foreach (TiaSound::Distortion distortion, availableWaveforms) {
        cbWaveforms->addItem(TiaSound::getDistortionName(distortion));
    }
    cbWaveforms->setCurrentIndex(10);   // Init to PURE_COMBINED

    // Volume shaper
    EnvelopeShaper *vs = findChild<EnvelopeShaper *>("volumeShaper");
    vs->registerInstrument(&(pTrack->instruments[0]));
    vs->name = "Volume";
    vs->setScale(0, 15);
    vs->setValues(pTrack->instruments[0].volumes);
    QObject::connect(vs, SIGNAL(valuesChanged(const QList<int>&)), this, SLOT(volumesChanged(const QList<int>&)));

    // Frequency shaper
    EnvelopeShaper *fs = findChild<EnvelopeShaper *>("frequencyShaper");
    fs->registerInstrument(&(pTrack->instruments[0]));
    fs->name = "Frequency";
    fs->setScale(-8, 7);
    fs->isInverted = true;
    fs->setValues(pTrack->instruments[0].frequencies);
    QObject::connect(fs, SIGNAL(valuesChanged(const QList<int>&)), this, SLOT(frequenciesChanged(const QList<int>&)));
}

/*************************************************************************/

void InstrumentsTab::updateInstrumentsTab() {
    assert(pTrack != nullptr);

    /* Global values */
    // Number of envelope frames used
    QLabel *lWaveformsUsed = findChild<QLabel *>("labelWaveformFramesUsed");
    int framesUsed = pTrack->getNumUsedEnvelopeFrames();
    QString framesUsedString;
    if (framesUsed < 256) {
        framesUsedString = "(" + QString::number(framesUsed) + " of 256 used)";

    } else {
        framesUsedString = "<font color=\"#dc322f\">(" + QString::number(framesUsed) + " of 256 used)</>";
    }
    lWaveformsUsed->setText(framesUsedString);
    // Number of instruments used
    QLabel *lInstrumentsUsed = findChild<QLabel *>("labelInstrumentsUsed");
    int instrumentsUsed = pTrack->getNumUsedInstruments();
    QString instrumentsUsedString;
    if (instrumentsUsed <= 7) {
        instrumentsUsedString = "(" + QString::number(instrumentsUsed) + " of 7 used)";
    } else {
        instrumentsUsedString = "<font color=\"#dc322f\">(" + QString::number(instrumentsUsed) + " of 7 used)</>";
    }
    lInstrumentsUsed->setText(instrumentsUsedString);
    // Names
    int iCurInstrument = getSelectedInstrumentIndex();
    QComboBox *cbInstruments = findChild<QComboBox *>("comboBoxInstruments");
    if (!cbInstruments->lineEdit()->hasFocus()) {
        cbInstruments->blockSignals(true);
        for (int ins = 0; ins < pTrack->numInstruments; ++ins) {
            cbInstruments->setItemText(ins, pTrack->instruments[ins].name);
        }
        // special requirement to update a QComboBox
        // when signals are disabled:
        cbInstruments->setCurrentIndex(iCurInstrument);
        cbInstruments->blockSignals(false);
    }

    /* Values specific to the selected intrument */
    QLabel *lInstrumentNumber = findChild<QLabel *>("labelInstrumentNumber");
    lInstrumentNumber->setText("Instrument " + QString::number(iCurInstrument + 1));
    Track::Instrument& curInstrument = pTrack->instruments[iCurInstrument];
    // Envelope length
    QSpinBox *spEnvelopeLength = findChild<QSpinBox *>("spinBoxInstrumentEnvelopeLength");
    int envelopeLength = curInstrument.getEnvelopeLength();
    spEnvelopeLength->blockSignals(true);
    spEnvelopeLength->setValue(envelopeLength);
    spEnvelopeLength->blockSignals(false);
    // Sustain and release start values
    QSpinBox *spSustainStart = findChild<QSpinBox *>("spinBoxSustainStart");
    int sustainStart = curInstrument.getSustainStart();
    spSustainStart->blockSignals(true);
    spSustainStart->setValue(sustainStart + 1);
    spSustainStart->blockSignals(false);
    QSpinBox *spReleaseStart = findChild<QSpinBox *>("spinBoxReleaseStart");
    int releaseStart = curInstrument.getReleaseStart();
    spReleaseStart->blockSignals(true);
    spReleaseStart->setValue(releaseStart + 1);
    spReleaseStart->blockSignals(false);
    // Peak volume
    QSpinBox *spPeakVolume = findChild<QSpinBox *>("spinBoxInstrumentVolume");
    int maxVolume = curInstrument.getMaxVolume();
    spPeakVolume->blockSignals(true);
    spPeakVolume->setValue(maxVolume);
    spPeakVolume->blockSignals(false);
    // Base waveform
    TiaSound::Distortion curDistortion = curInstrument.baseDistortion;
    int iWaveform = availableWaveforms.indexOf(curDistortion);
    assert(iWaveform != -1);
    QComboBox *cbWaveforms = findChild<QComboBox *>("comboBoxWaveforms");
    cbWaveforms->blockSignals(true);
    cbWaveforms->setCurrentIndex(iWaveform);
    cbWaveforms->blockSignals(false);
    emit setWaveform(curInstrument.baseDistortion);
    // EnvelopeShaper sizes and values
    EnvelopeShaper *wsVolume = findChild<EnvelopeShaper *>("volumeShaper");
    wsVolume->registerInstrument(&curInstrument);
    wsVolume->setValues(curInstrument.volumes);
    wsVolume->updateSize();
    EnvelopeShaper *wsFrequency = findChild<EnvelopeShaper *>("frequencyShaper");
    wsFrequency->registerInstrument(&curInstrument);
    wsFrequency->setValues(curInstrument.frequencies);
    wsFrequency->updateSize();
}

/*************************************************************************/

int InstrumentsTab::getSelectedInstrumentIndex() {
    QComboBox *cbInstruments = findChild<QComboBox *>("comboBoxInstruments");
    return cbInstruments->currentIndex();
}

/*************************************************************************/

Track::Instrument * InstrumentsTab::getSelectedInstrument() {
    int iCurInstrument = getSelectedInstrumentIndex();
    Track::Instrument *curInstrument = &(pTrack->instruments[iCurInstrument]);
    return curInstrument;
}

/*************************************************************************/

void InstrumentsTab::on_buttonInstrumentDelete_clicked() {
    Track::Instrument *curInstrument = getSelectedInstrument();
    bool doDelete = true;
    if (!curInstrument->isEmpty()) {
        QMessageBox msgBox(QMessageBox::NoIcon,
                           "Delete Instrument",
                           "Do you really want to delete this instument?",
                           QMessageBox::Yes | QMessageBox::No, this,
                           Qt::FramelessWindowHint);
        int reply = msgBox.exec();
        if (reply != QMessageBox::Yes) {
            doDelete = false;
        }
    }
    if (!doDelete)
        return;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetInstrumentCommand(pTrack, getSelectedInstrumentIndex(), Track::Instrument("---"));

    cmd->setText("Delete Instrument " + curInstrument->name);

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.instrumentTab = true;

    undoStack->push(cmd);
}

/*************************************************************************/

void InstrumentsTab::on_buttonInstrumentExport_clicked() {
    Track::Instrument *curInstrument = getSelectedInstrument();

    if (curInstrument->isEmpty()) {
        return;
    }

    // Ask for filename
    QFileDialog dialog(this);
    dialog.setDirectory(curInstrumentsDialogPath);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("*.tti");
    dialog.setDefaultSuffix("tti");
    dialog.setViewMode(QFileDialog::Detail);
    dialog.selectFile(curInstrument->name);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    }
    if (fileNames.isEmpty()) {
        return;
    }
    QString fileName = fileNames[0];
    curInstrumentsDialogPath = dialog.directory().absolutePath();
    QFile saveFile(fileName);

    // Export instrument
    if (!saveFile.open(QIODevice::WriteOnly)) {
        MainWindow::displayMessage("Unable to open file!");
        return;
    }
    QJsonObject insObject;
    curInstrument->toJson(insObject);
    QJsonDocument saveDoc(insObject);
    saveFile.write(saveDoc.toJson());
    saveFile.close();
}

/*************************************************************************/

void InstrumentsTab::on_buttonInstrumentImport_clicked() {
    Track::Instrument *curInstrument = getSelectedInstrument();

    // Ask if instrument should really be overwritten
    bool doImport = true;
    if (!curInstrument->isEmpty()) {
        QMessageBox msgBox(QMessageBox::NoIcon,
                           "Import Instrument",
                           "Do you really want to overwrite the current instument?",
                           QMessageBox::Yes | QMessageBox::No, this,
                           Qt::FramelessWindowHint);
        int reply = msgBox.exec();
        if (reply != QMessageBox::Yes) {
            doImport = false;
        }
    }
    if (!doImport) {
        return;
    }

    // Ask for filename
    QFileDialog dialog(this);
    dialog.setDirectory(curInstrumentsDialogPath);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("*.tti");
    dialog.setDefaultSuffix("tti");
    dialog.setViewMode(QFileDialog::Detail);

    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    }
    if (fileNames.isEmpty()) {
        return;
    }
    QString fileName = fileNames[0];
    curInstrumentsDialogPath = dialog.directory().absolutePath();
    QFile loadFile(fileName);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        MainWindow::displayMessage("Unable to open file!");
        return;
    }
    QJsonDocument loadDoc(QJsonDocument::fromJson(loadFile.readAll()));

    Track::Instrument inst(curInstrument->name);
    // Parse in data
    if (!inst.import(loadDoc.object())) {
        MainWindow::displayMessage("Unable to parse instrument!");
        return;
    }

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    QString instName = inst.name;

    auto cmd = new SetInstrumentCommand(pTrack, getSelectedInstrumentIndex(), std::move(inst));

    cmd->setText("Import Instrument " + instName);

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.instrumentTab = true;

    undoStack->push(cmd);
}

/*************************************************************************/

void InstrumentsTab::on_spinBoxInstrumentEnvelopeLength_valueChanged(int newLength) {
    Track::Instrument *curInstrument = getSelectedInstrument();
    if (std::abs(newLength - curInstrument->getEnvelopeLength()) != 1)
        return;

    Track::Instrument inst = *curInstrument;
    inst.setEnvelopeLength(newLength);

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetInstrumentCommand(pTrack, getSelectedInstrumentIndex(), std::move(inst));

    cmd->setText("Set Instrument Envelope Length");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.instrumentTab = true;

    undoStack->push(cmd);
}

/*************************************************************************/

void InstrumentsTab::on_spinBoxSustainStart_valueChanged(int newStart) {
    newStart--;
    Track::Instrument *curInstrument = getSelectedInstrument();
    if (std::abs(newStart - curInstrument->getSustainStart()) != 1)
        return;
    
    Track::Instrument inst = *curInstrument;
    {
        QSpinBox *sb = findChild<QSpinBox *>("spinBoxSustainStart");
        int newStart = sb->value() - 1;
        if (newStart < inst.getReleaseStart()) {
            // valid new value
            inst.setSustainAndRelease(newStart, inst.getReleaseStart());
        }
        else {
            // invalid new value. Try to push release start
            int newRelease = newStart + 1;
            if (newRelease < inst.getEnvelopeLength()) {
                inst.setSustainAndRelease(newStart, newRelease);
            }
            else {
                // Release start cannot be pushed ahead, so reject new sustain value
                sb->blockSignals(true);
                sb->setValue(inst.getSustainStart() + 1);
                sb->blockSignals(false);
                return;
            }
        }
    }

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetInstrumentCommand(pTrack, getSelectedInstrumentIndex(), std::move(inst));

    cmd->setText("Set Instrument Sustain Start");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.instrumentTab = true;

    undoStack->push(cmd);
}

/*************************************************************************/

void InstrumentsTab::on_spinBoxReleaseStart_valueChanged(int newStart) {
    newStart--;
    Track::Instrument *curInstrument = getSelectedInstrument();
    if (std::abs(newStart - curInstrument->getReleaseStart()) != 1)
        return;

    Track::Instrument inst = *curInstrument;
    {
        QSpinBox *sb = findChild<QSpinBox *>("spinBoxReleaseStart");
        int newStart = sb->value() - 1;
        if (newStart < inst.getEnvelopeLength()
            && newStart > inst.getSustainStart()) {
            // valid new value
            inst.setSustainAndRelease(inst.getSustainStart(), newStart);
        }
        else {
            // invalid new value
            sb->blockSignals(true);
            sb->setValue(inst.getReleaseStart() + 1);
            sb->blockSignals(false);
            return;
        }
    }

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetInstrumentCommand(pTrack, getSelectedInstrumentIndex(), std::move(inst));

    cmd->setText("Set Instrument Release Start");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.instrumentTab = true;

    undoStack->push(cmd);
}

/*************************************************************************/

void InstrumentsTab::on_spinBoxInstrumentVolume_valueChanged(int newVolume) {
    Track::Instrument *curInstrument = getSelectedInstrument();
    if (std::abs(newVolume - curInstrument->getMaxVolume()) != 1)
        return;

    Track::Instrument inst = *curInstrument;   
    {
        QSpinBox *sb = findChild<QSpinBox *>("spinBoxInstrumentVolume");
        int newVolume = sb->value();
        int curMin = inst.getMinVolume();
        int curMax = inst.getMaxVolume();
        int curVolumeSpan = curMax - curMin;
        if (newVolume - curVolumeSpan >= 0) {
            // Shift volumes
            int volumeShift = newVolume - curMax;
            for (int i = 0; i < inst.getEnvelopeLength(); ++i) {
                inst.volumes[i] += volumeShift;
            }
        }
        else {
            // Invalid value: Set volume to current max
            sb->blockSignals(true);
            sb->setValue(inst.getMaxVolume());
            sb->blockSignals(false);
            return;
        }
    }

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetInstrumentCommand(pTrack, getSelectedInstrumentIndex(), std::move(inst));

    cmd->setText("Set Instrument Peak volume");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.instrumentTab = true;

    undoStack->push(cmd);
}

/*************************************************************************/

void InstrumentsTab::on_comboBoxWaveforms_currentIndexChanged(int index) {
    Track::Instrument *curInstrument = getSelectedInstrument();
    TiaSound::Distortion newDistortion = availableWaveforms[index];

    Track::Instrument inst = *curInstrument;
    inst.baseDistortion = newDistortion;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetInstrumentCommand(pTrack, getSelectedInstrumentIndex(), std::move(inst));

    cmd->setText("Set Instrument Base waveform");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.instrumentTab = true;

    undoStack->push(cmd);
}

/*************************************************************************/

void InstrumentsTab::on_comboBoxInstruments_currentIndexChanged(int) {
    updateInstrumentsTab();
    update();
}

/*************************************************************************/

void InstrumentsTab::on_comboBoxInstruments_editTextChanged(const QString &text) {
    Track::Instrument *curInstrument = getSelectedInstrument();

    Track::Instrument inst = *curInstrument;
    inst.name = text;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    if (!bStartNameEditing) {
        iStartNameEditCount = undoStack->count();
        bStartNameEditing = true;
    }

    auto cmd = new SetInstrumentCommand(pTrack, getSelectedInstrumentIndex(), std::move(inst),
        /*mergeable =*/ true);

    cmd->setText("Set Instrument Name");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.instrumentTab = true;

    undoStack->push(cmd);

    if (undoStack->count() == iStartNameEditCount) {
        bStartNameEditing = false;
    }
}

/*************************************************************************/

void InstrumentsTab::on_comboBoxInstruments_editingFinished()
{
    if (bStartNameEditing) {
        SetInstrumentCommand::ToggleID();
        bStartNameEditing = false;
    }

    setFocus(); // we want the QLineEdit/QPlainTextEdit that was edited to loose the focus
}

/*************************************************************************/

void InstrumentsTab::volumesChanged(const QList<int>& volumes)
{
    Track::Instrument *curInstrument = getSelectedInstrument();

    Track::Instrument inst = *curInstrument;
    inst.volumes = volumes;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetInstrumentCommand(pTrack, getSelectedInstrumentIndex(), std::move(inst));

    cmd->setText("Edit Instrument Volumes");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.instrumentTab = true;

    undoStack->push(cmd);
}

/*************************************************************************/

void InstrumentsTab::frequenciesChanged(const QList<int>& frequencies)
{
    Track::Instrument *curInstrument = getSelectedInstrument();

    Track::Instrument inst = *curInstrument;
    inst.frequencies = frequencies;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetInstrumentCommand(pTrack, getSelectedInstrumentIndex(), std::move(inst));

    cmd->setText("Edit Instrument Frequencies");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.instrumentTab = true;

    undoStack->push(cmd);
}
