/* TIATracker, (c) 2016 Andre "Kylearan" Wichmann.
 * Website: https://bitbucket.org/kylearan/tiatracker
 * Email: andre.wichmann@gmx.de
 * See the file "license.txt" for information on usage and redistribution
 * of this file.
 */

#include "percussiontab.h"
#include <QComboBox>
#include <QLineEdit>
#include <cassert>
#include <QLabel>
#include <QSpinBox>
#include "envelopeshaper.h"
#include "percussionshaper.h"
#include "waveformshaper.h"
#include <QMessageBox>
#include <iostream>
#include <QFileDialog>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "mainwindow.h"
#include "track/percussion.h"
#include <QCheckBox>


const QList<TiaSound::Distortion> PercussionTab::availableWaveforms{
    TiaSound::Distortion::SILENT,
    TiaSound::Distortion::BUZZY,
    TiaSound::Distortion::BUZZY_RUMBLE,
    TiaSound::Distortion::FLANGY_WAVERING,
    TiaSound::Distortion::PURE_HIGH,
    TiaSound::Distortion::PURE_BUZZY,
    TiaSound::Distortion::REEDY_RUMBLE,
    TiaSound::Distortion::WHITE_NOISE,
    TiaSound::Distortion::PURE_LOW,
    TiaSound::Distortion::ELECTRONIC_RUMBLE,
    TiaSound::Distortion::ELECTRONIC_SQUEAL
};

/*************************************************************************/

PercussionTab::PercussionTab(QWidget *parent) : QWidget(parent)
{

}

/*************************************************************************/

void PercussionTab::registerTrack(Track::Track *newTrack) {
    pTrack = newTrack;
}

/*************************************************************************/

void PercussionTab::initPercussionTab() {
    // Percussion names
    QComboBox *cbPercussion = findChild<QComboBox *>("comboBoxPercussion");
    cbPercussion->lineEdit()->setMaxLength(maxPercussionNameLength);
    foreach(Track::Percussion perc, pTrack->percussion) {
        cbPercussion->addItem(perc.name);
    }

    // Volume shaper
    PercussionShaper *vs = findChild<PercussionShaper *>("percussionVolumeShaper");
    vs->registerPercussion(&(pTrack->percussion[0]));
    vs->name = "Volume";
    vs->setScale(0, 15);
    vs->setValues(pTrack->percussion[0].volumes);
    QObject::connect(vs, SIGNAL(valuesChanged(const QList<int>&)), this, SLOT(volumeChanged(const QList<int>&)));

    // Frequency shaper
    PercussionShaper *fs = findChild<PercussionShaper *>("percussionFrequencyShaper");
    fs->registerPercussion(&(pTrack->percussion[0]));
    fs->name = "Frequency";
    fs->setScale(0, 31);
    fs->isInverted = true;
    fs->setValues(pTrack->percussion[0].frequencies);
    QObject::connect(fs, SIGNAL(valuesChanged(const QList<int>&)), this, SLOT(frequencyChanged(const QList<int>&)));

    // Waveform shaper
    WaveformShaper *ws = findChild<WaveformShaper *>("percussionWaveformShaper");
    ws->registerPercussion(&(pTrack->percussion[0]));
    ws->setValues(pTrack->percussion[0].waveforms);
    QObject::connect(ws, SIGNAL(valuesChanged(const QList<TiaSound::Distortion>&)), this, SLOT(waveformChanged(const QList<TiaSound::Distortion>&)));
}

/*************************************************************************/

void PercussionTab::updatePercussionTab() {
    /* Global values */
    // Number of envelope frames used
    QLabel *lFramesUsed = findChild<QLabel *>("labelPercussionFramesUsed");
    int framesUsed = pTrack->getNumUsedPercussionFrames();
    QString framesUsedString;
    if (framesUsed < 256) {
        framesUsedString = "(" + QString::number(framesUsed) + " of 256 used)";

    } else {
        framesUsedString = "<font color=\"#dc322f\">(" + QString::number(framesUsed) + " of 256 used)</>";
    }
    lFramesUsed->setText(framesUsedString);
    // Number of percussion used
    QLabel *lPercussionUsed = findChild<QLabel *>("labelPercussionUsed");
    int percussionUsed = pTrack->getNumUsedPercussion();
    QString percussionUsedString = "(" + QString::number(percussionUsed) + " of 15 used)";
    lPercussionUsed->setText(percussionUsedString);
    // Names
    int iCurPercussion = getSelectedPercussionIndex();
    QComboBox *cbPercussion = findChild<QComboBox *>("comboBoxPercussion");
    if (!cbPercussion->lineEdit()->hasFocus()) {
        cbPercussion->blockSignals(true);
        for (int perc = 0; perc < pTrack->numPercussion; ++perc) {
            cbPercussion->setItemText(perc, pTrack->percussion[perc].name);
        }
        // special requirement to update a QComboBox
        // when signals are disabled:
        cbPercussion->setCurrentIndex(iCurPercussion);
        cbPercussion->blockSignals(false);
    }
    /* Values specific to the selected percussion */
    QLabel *lPercussionNumber = findChild<QLabel *>("labelPercussionNumber");
    lPercussionNumber->setText("Percussion " + QString::number(iCurPercussion + 1));
    Track::Percussion& curPercussion = pTrack->percussion[iCurPercussion];
    // Envelope length
    QSpinBox *spPercussionLength = findChild<QSpinBox *>("spinBoxPercussionLength");
    int envelopeLength = curPercussion.getEnvelopeLength();
    spPercussionLength->blockSignals(true);
    spPercussionLength->setValue(envelopeLength);
    spPercussionLength->blockSignals(false);
    // Peak volume
    QSpinBox *spPeakVolume = findChild<QSpinBox *>("spinBoxPercussionVolume");
    int maxVolume = curPercussion.getMaxVolume();
    spPeakVolume->blockSignals(true);
    spPeakVolume->setValue(maxVolume);
    spPeakVolume->blockSignals(false);
    // Overlay
    QCheckBox *cpOverlay = findChild<QCheckBox *>("checkBoxOverlay");
    cpOverlay->blockSignals(true);
    cpOverlay->setChecked(curPercussion.overlay);
    cpOverlay->blockSignals(false);
    // PercussionShaper sizes and values
    PercussionShaper *psVolume = findChild<PercussionShaper *>("percussionVolumeShaper");
    psVolume->registerPercussion(&curPercussion);
    psVolume->setValues(curPercussion.volumes);
    psVolume->updateSize();
    PercussionShaper *psFrequency = findChild<PercussionShaper *>("percussionFrequencyShaper");
    psFrequency->registerPercussion(&curPercussion);
    psFrequency->setValues(curPercussion.frequencies);
    psFrequency->updateSize();
    WaveformShaper *wsWaveforms = findChild<WaveformShaper *>("percussionWaveformShaper");
    wsWaveforms->registerPercussion(&curPercussion);
    wsWaveforms->setValues(curPercussion.waveforms);
    wsWaveforms->updateSize();
}

/*************************************************************************/

void PercussionTab::connectPlayer(Emulation::Player *player) {
    PercussionShaper *psVolume = findChild<PercussionShaper *>("percussionVolumeShaper");
    QObject::connect(psVolume, SIGNAL(silence()), player, SLOT(silence()));
    PercussionShaper *psFrequency = findChild<PercussionShaper *>("percussionFrequencyShaper");
    QObject::connect(psFrequency, SIGNAL(silence()), player, SLOT(silence()));
}

/*************************************************************************/

int PercussionTab::getSelectedPercussionIndex() {
    QComboBox *cbPercussion = findChild<QComboBox *>("comboBoxPercussion");
    return cbPercussion->currentIndex();
}

/*************************************************************************/

void PercussionTab::on_buttonPercussionExport_clicked() {
    Track::Percussion *curPercussion = getSelectedPercussion();

    if (curPercussion->isEmpty()) {
        return;
    }

    // Ask for filename
    QFileDialog dialog(this);
    dialog.setDirectory(curPercussionDialogPath);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("*.ttp");
    dialog.setDefaultSuffix("ttp");
    dialog.setViewMode(QFileDialog::Detail);
    dialog.selectFile(curPercussion->name);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    }
    if (fileNames.isEmpty()) {
        return;
    }
    QString fileName = fileNames[0];
    curPercussionDialogPath = dialog.directory().absolutePath();
    QFile saveFile(fileName);

    // Export Percussion
    if (!saveFile.open(QIODevice::WriteOnly)) {
        MainWindow::displayMessage("Unable to open file!");
        return;
    }
    QJsonObject insObject;
    curPercussion->toJson(insObject);
    QJsonDocument saveDoc(insObject);
    saveFile.write(saveDoc.toJson());
    saveFile.close();
}

/*************************************************************************/

void PercussionTab::on_buttonPercussionImport_clicked() {
    Track::Percussion *curPercussion = getSelectedPercussion();

    // Ask if Percussion should really be overwritten
    bool doImport = true;
    if (!curPercussion->isEmpty()) {
        QMessageBox msgBox(QMessageBox::NoIcon,
                           "Import Percussion",
                           "Do you really want to overwrite the current percussion?",
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
    dialog.setDirectory(curPercussionDialogPath);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("*.ttp");
    dialog.setDefaultSuffix("ttp");
    dialog.setViewMode(QFileDialog::Detail);

    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    }
    if (fileNames.isEmpty()) {
        return;
    }
    QString fileName = fileNames[0];
    curPercussionDialogPath = dialog.directory().absolutePath();
    QFile loadFile(fileName);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        MainWindow::displayMessage("Unable to open file!");
        return;
    }
    QJsonDocument loadDoc(QJsonDocument::fromJson(loadFile.readAll()));

    Track::Percussion perc(curPercussion->name);
    // Parse in data
    if (!perc.import(loadDoc.object())) {
        MainWindow::displayMessage("Unable to parse percussion!");
        return;
    }

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    QString percName = perc.name;

    auto cmd = new SetPercussionCommand(pTrack, getSelectedPercussionIndex(), std::move(perc));

    cmd->setText("Import Percussion " + percName);

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabPercussion;

    undoStack->push(cmd);
}

/*************************************************************************/

Track::Percussion *PercussionTab::getSelectedPercussion() {
    int iCurPercussion = getSelectedPercussionIndex();
    Track::Percussion *curPercussion = &(pTrack->percussion[iCurPercussion]);
    return curPercussion;
}

/*************************************************************************/

void PercussionTab::on_buttonPercussionDelete_clicked() {
    Track::Percussion *curPercussion = getSelectedPercussion();
    bool doDelete = true;
    if (!curPercussion->isEmpty()) {
        QMessageBox msgBox(QMessageBox::NoIcon,
                           "Delete Percussion",
                           "Do you really want to delete this percussion?",
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

    auto cmd = new SetPercussionCommand(pTrack, getSelectedPercussionIndex(), Track::Percussion("---"));

    cmd->setText("Delete Percussion " + curPercussion->name);

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabPercussion;

    undoStack->push(cmd);
}

/*************************************************************************/

void PercussionTab::on_spinBoxPercussionLength_valueChanged(int newLength) {
    Track::Percussion *curPercussion = getSelectedPercussion();
    if (std::abs(newLength - curPercussion->getEnvelopeLength()) != 1)
        return;

    Track::Percussion perc = *curPercussion;
    perc.setEnvelopeLength(newLength);

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetPercussionCommand(pTrack, getSelectedPercussionIndex(), std::move(perc));

    cmd->setText("Set Percussion Envelope length");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabPercussion;

    undoStack->push(cmd);
}

/*************************************************************************/

void PercussionTab::on_checkBoxOverlay_stateChanged(int state) {

    Track::Percussion *curPercussion = getSelectedPercussion();

    Track::Percussion perc = *curPercussion;
    perc.overlay = state != Qt::Unchecked;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetPercussionCommand(pTrack, getSelectedPercussionIndex(), std::move(perc));

    cmd->setText("Set Percussion Overlay");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabPercussion;

    undoStack->push(cmd);
}

/*************************************************************************/

void PercussionTab::on_spinBoxPercussionVolume_valueChanged(int newVolume) {
    Track::Percussion *curPercussion = getSelectedPercussion();
    if (std::abs(newVolume - curPercussion->getMaxVolume()) != 1)
        return;

    QSpinBox *sb = findChild<QSpinBox *>("spinBoxPercussionVolume");

    Track::Percussion perc = *curPercussion;

    int curMin = perc.getMinVolume();
    int curMax = perc.getMaxVolume();
    int curVolumeSpan = curMax - curMin;
    if (newVolume - curVolumeSpan >= 0) {
        // Shift volumes
        int volumeShift = newVolume - curMax;
        for (int i = 0; i < perc.getEnvelopeLength(); ++i) {
            perc.volumes[i] += volumeShift;
        }
    }
    else {
        // Invalid value: Set volume to current max
        sb->blockSignals(true);
        sb->setValue(perc.getMaxVolume());
        sb->blockSignals(false);
        return;
    }

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetPercussionCommand(pTrack, getSelectedPercussionIndex(), std::move(perc));

    cmd->setText("Set Percussion Volume");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabPercussion;

    undoStack->push(cmd);
}

/*************************************************************************/

void PercussionTab::on_comboBoxPercussion_currentIndexChanged(int) {
    updatePercussionTab();
    update();
}

/*************************************************************************/

void PercussionTab::on_comboBoxPercussion_editTextChanged(const QString &text) {
    Track::Percussion *curPercussion = getSelectedPercussion();

    Track::Percussion perc = *curPercussion;
    perc.name = text;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    if (!bStartNameEditing) {
        iStartNameEditCount = undoStack->count();
        bStartNameEditing = true;
    }

    auto cmd = new SetPercussionCommand(pTrack, getSelectedPercussionIndex(), std::move(perc),
        /*mergeable =*/ true);

    cmd->setText("Set Percussion Name");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabPercussion;

    undoStack->push(cmd);

    if (undoStack->count() == iStartNameEditCount) {
        bStartNameEditing = false;
    }
}

/*************************************************************************/

void PercussionTab::newPercussionValue(int iFrame) {
    Track::Percussion *curPercussion = getSelectedPercussion();
    TiaSound::Distortion waveform = curPercussion->waveforms[iFrame];
    int frequency = curPercussion->frequencies[iFrame];
    int volume = curPercussion->volumes[iFrame];
    emit playWaveform(waveform, frequency, volume);
}

/*************************************************************************/

void PercussionTab::on_comboBoxPercussion_editingFinished()
{
    if (bStartNameEditing) {
        SetPercussionCommand::ToggleID();
        bStartNameEditing = false;
    }

    setFocus(); // we want the QLineEdit/QPlainTextEdit that was edited to loose the focus
}

/*************************************************************************/

void PercussionTab::volumeChanged(const QList<int>& volumes)
{
    Track::Percussion *curPercussion = getSelectedPercussion();
    
    Track::Percussion perc = *curPercussion;
    perc.volumes = volumes;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetPercussionCommand(pTrack, getSelectedPercussionIndex(), std::move(perc));

    cmd->setText("Edit Percussion Volumes");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabPercussion;

    undoStack->push(cmd);
}

/*************************************************************************/

void PercussionTab::frequencyChanged(const QList<int>& frequencies)
{
    Track::Percussion *curPercussion = getSelectedPercussion();

    Track::Percussion perc = *curPercussion;
    perc.frequencies = frequencies;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetPercussionCommand(pTrack, getSelectedPercussionIndex(), std::move(perc));

    cmd->setText("Edit Percussion Frequencies");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabPercussion;

    undoStack->push(cmd);
}

/*************************************************************************/

void PercussionTab::waveformChanged(const QList<TiaSound::Distortion>& waveforms)
{
    Track::Percussion *curPercussion = getSelectedPercussion();

    Track::Percussion perc = *curPercussion;
    perc.waveforms = waveforms;

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto cmd = new SetPercussionCommand(pTrack, getSelectedPercussionIndex(), std::move(perc));

    cmd->setText("Edit Percussion Frequencies");

    cmd->post = this->window()->findChild<UndoStep*>("TabsUpdate");

    cmd->ci.tab = MainWindow::iTabPercussion;

    undoStack->push(cmd);
}
