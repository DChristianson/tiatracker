#include "createguidedialog.h"
#include "ui_createguidedialog.h"

#include <QMap>
#include <QString>
#include "tiasound/tiasound.h"
#include "tiasound/pitchguidefactory.h"
#include "percussiontab.h"
#include <iostream>
#include "mainwindow.h"
#include <QProgressDialog>
#include "guidekeyboard.h"
#include <QKeyEvent>
#include <cmath>


CreateGuideDialog::CreateGuideDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateGuideDialog)
{
    ui->setupUi(this);
    // Fill waveform selector
    for (int i = 0; i < checkBoxNames.keys().size(); ++i) {
        TiaSound::Distortion dist = checkBoxNames.keys()[i];
        ui->comboBoxGuideWaveforms->addItem(TiaSound::getDistortionName(dist));
    }
}

CreateGuideDialog::~CreateGuideDialog()
{
    delete ui;
}

/*************************************************************************/

void CreateGuideDialog::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        break;

    default:
        QDialog::keyPressEvent (e);
    }
}

/*************************************************************************/

void CreateGuideDialog::on_pushButtonCreateGuide_clicked() {
    int threshold = ui->spinBoxGuideMaxOffTune->value();
    QString name = ui->lineEditGuideName->text();
    TiaSound::TvStandard standard = ui->radioButtonGuidePal->isChecked() ? TiaSound::TvStandard::PAL : TiaSound::TvStandard::NTSC;
    TiaSound::PitchGuideFactory pg;

    // Optimize
    long bestError = 99999999;
    double bestFreq = 0;
    int bestNum = 0;
    int maxNoteDeviation = ui->spinBoxGuideMaxDeviation->value();
    double lowerBound = std::pow(2.0, -maxNoteDeviation/12.0) * 440.0;
    double upperBound = std::pow(2.0, maxNoteDeviation/12.0) * 440.0;

    // Loop over frequencies in range
    QProgressDialog pd("Optimizing pitch guide...", "Cancel", 0, (upperBound - lowerBound)*10, this);
    pd.setWindowModality(Qt::WindowModal);
    pd.setMinimumDuration(0);
    GuideKeyboard *keyboard = findChild<GuideKeyboard *>("guidePianoKeyboard");

    for (double f = lowerBound; f < upperBound; f += 0.1) {
        pd.setValue((f - lowerBound)*10);
        long currentError = 0;
        int curNum = 0;
        // For this frequency, loop over all requested distortions
        for (int i = 0; i < checkBoxNames.size(); ++i) {
            TiaSound::Distortion dist = checkBoxNames.keys()[i];
            QCheckBox *cbDist = findChild<QCheckBox *>(checkBoxNames[dist]);
            if (cbDist->isChecked()) {
                QList<TiaSound::FrequencyPitchGuide> guide = pg.calcInstrumentPitchGuide(standard, dist, f);
                for (int i = 0; i < guide.size(); ++i) {
                    // Consider only requested notes
                    if (guide[i].note != TiaSound::Note::NotANote) {
                        int noteIndex = TiaSound::getIntFromNote(guide[i].note);
                        if (keyboard->keyInfo[noteIndex].isEnabled
                                && std::abs(guide[i].percentOff) <= threshold) {
                            currentError += guide[i].percentOff*guide[i].percentOff;
                            curNum++;
                        }
                    }
                }
            }
        }
        // Now check if new best has been found
        if (curNum > bestNum || (curNum == bestNum && currentError < bestError)) {
            bestError = currentError;
            bestFreq = f;
            bestNum = curNum;
        }
        // Cancel pressed?
        if (pd.wasCanceled()) {
            ui->labelGuideBaseFreq->setText("");
            isGuideCreated = false;
            ui->guidePianoKeyboard->removeGuide();
            update();
            return;
        }
    }

    // Now create pitch guide for the best frequency
    newGuide = pg.calculateGuide(name, standard, bestFreq);
    isGuideCreated = true;

    // Set A4= label text
    ui->labelGuideBaseFreq->setText("A4 = " + QString::number(newGuide.baseFreq) + "Hz.");
    on_comboBoxGuideWaveforms_currentIndexChanged(ui->comboBoxGuideWaveforms->currentIndex());
}

/*************************************************************************/

void CreateGuideDialog::on_comboBoxGuideWaveforms_currentIndexChanged(int index) {
    if (isGuideCreated) {
        TiaSound::Distortion dist = checkBoxNames.keys()[index];
        TiaSound::InstrumentPitchGuide *guide = &(newGuide.instrumentGuides[dist]);
        int threshold = ui->spinBoxGuideMaxOffTune->value();
        ui->guidePianoKeyboard->setInstrumentPitchGuide(guide, threshold);
        update();
    }
}

/*************************************************************************/

void CreateGuideDialog::on_lineEditGuideName_editingFinished() {
    if (isGuideCreated) {
        newGuide.name = ui->lineEditGuideName->text();
    }
}
