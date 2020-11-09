#include "optionstab.h"
#include <QComboBox>
#include <QSpinBox>
#include "createguidedialog.h"
#include <QLayout>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include "mainwindow.h"
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QPlainTextEdit>
#include <QTextDocument>


OptionsTab::OptionsTab(QWidget *parent) : QWidget(parent)
{
    TiaSound::PitchGuideFactory pgFactory;
    guides.append(pgFactory.getPitchPerfectPalGuide());
    guides.append(pgFactory.getOptimizedPurePalGuide());
    guides.append(pgFactory.getOptimizedDiv232PalGuide());
    guides.append(pgFactory.getOptimizedDiv93PalGuide());
    guides.append(pgFactory.getOptimizedDiv31PalGuide());
    guides.append(pgFactory.getOptimizedDiv15PalGuide());
    guides.append(pgFactory.getOptimizedDiv6PalGuide());
    guides.append(pgFactory.getOptimizedDiv2PalGuide());
    guides.append(pgFactory.getPitchPerfectNtscGuide());
    guides.append(pgFactory.getOptimizedPureNtscGuide());
    guides.append(pgFactory.getOptimizedDiv232NtscGuide());
    guides.append(pgFactory.getOptimizedDiv93NtscGuide());
    guides.append(pgFactory.getOptimizedDiv31NtscGuide());
    guides.append(pgFactory.getOptimizedDiv15NtscGuide());
    guides.append(pgFactory.getOptimizedDiv6NtscGuide());
    guides.append(pgFactory.getOptimizedDiv2NtscGuide());
}

/*************************************************************************/

void OptionsTab::registerTrack(Track::Track *newTrack) {
    pTrack = newTrack;
}

/*************************************************************************/

void OptionsTab::initOptionsTab() {
    QComboBox *cbGuides = findChild<QComboBox *>("comboBoxPitchGuide");
    for (int i = 0; i < guides.size(); ++i) {
        cbGuides->addItem(guides[i].name);
    }
}

/*************************************************************************/

void OptionsTab::updateOptionsTab() {
    // TvStandard
    QRadioButton *rbPal = findChild<QRadioButton *>("radioButtonPal");
    rbPal->blockSignals(true);
    QRadioButton *rbNtsc = findChild<QRadioButton *>("radioButtonNtsc");
    rbNtsc->blockSignals(true);
    if (pTrack->tvMode == TiaSound::TvStandard::PAL) {
        rbPal->setChecked(true);
        rbNtsc->setChecked(false);
        emit setTVStandard(static_cast<int>(TiaSound::TvStandard::PAL));
    } else {
        rbPal->setChecked(false);
        rbNtsc->setChecked(true);
        emit setTVStandard(static_cast<int>(TiaSound::TvStandard::NTSC));
    }
    rbPal->blockSignals(false);
    rbNtsc->blockSignals(false);
    // Meta data
    QLineEdit *leAuthor = findChild<QLineEdit *>("lineEditAuthor");
    leAuthor->blockSignals(true);
    leAuthor->setText(pTrack->metaAuthor);
    leAuthor->blockSignals(false);
    QLineEdit *leSongName = findChild<QLineEdit *>("lineEditSongName");
    leSongName->blockSignals(true);
    leSongName->setText(pTrack->metaName);
    leSongName->blockSignals(false);
    QPlainTextEdit *te = findChild<QPlainTextEdit *>("plainTextEditComment");
    te->blockSignals(true);
    te->setPlainText(pTrack->metaComment);
    te->blockSignals(false);

    // Pitch guide
    QLabel *infoLabel = findChild<QLabel *>("labelGuideInfo");
    QComboBox *cbGuides = findChild<QComboBox *>("comboBoxPitchGuide");
    TiaSound::PitchGuide *pg = &(guides[cbGuides->currentIndex()]);
    QString tvText = (pg->tvStandard == TiaSound::TvStandard::PAL ? "PAL" : "NTSC");
    infoLabel->setText("(" + tvText + ", " + QString::number(pg->baseFreq) + "Hz)");
}

/*************************************************************************/

void OptionsTab::on_comboBoxPitchGuide_currentIndexChanged(int index) {
    // Update guide data for track
    QComboBox *cbGuides = findChild<QComboBox *>("comboBoxPitchGuide");
    TiaSound::PitchGuide *pg = &(guides[cbGuides->currentIndex()]);
    pTrack->guideName = pg->name;
    pTrack->guideBaseFreq = pg->baseFreq;
    pTrack->guideTvStandard = pg->tvStandard;
    updateOptionsTab();
    emit setPitchGuide(guides[index]);
}

/*************************************************************************/

void OptionsTab::on_radioButtonPal_toggled(bool checked) {

    auto undoStack = this->window()->findChild<QUndoStack*>("UndoStack");

    auto tvMode = checked ? TiaSound::TvStandard::PAL : TiaSound::TvStandard::NTSC;

    auto cmd = new SetValueCommand<TiaSound::TvStandard>(pTrack, pTrack->tvMode, tvMode);

    cmd->setText(checked ? "Set TV Standard to PAL" : "Set TV Standard to NTSC");

    cmd->post = this->window()->findChild<UndoStep*>("TrackTabUpdate");
    
    cmd->ci.optionsTab = true;

    undoStack->push(cmd);
}

/*************************************************************************/

void OptionsTab::on_spinBoxOffTuneThreshold_editingFinished() {
    QSpinBox *sbThreshold = findChild<QSpinBox *>("spinBoxOffTuneThreshold");
    emit setOffTuneThreshold(sbThreshold->value());
}

/*************************************************************************/

void OptionsTab::addGuide(TiaSound::PitchGuide newGuide) {
    guides.append(newGuide);
    QComboBox *cbGuides = findChild<QComboBox *>("comboBoxPitchGuide");
    cbGuides->addItem(newGuide.name);
    cbGuides->setCurrentIndex(cbGuides->count() - 1);
    updateOptionsTab();
}

/*************************************************************************/

void OptionsTab::on_pushButtonGuideCreate_clicked(bool) {
    CreateGuideDialog newDialog(this);
    newDialog.layout()->setSizeConstraint(QLayout::SetFixedSize);
    if (newDialog.exec() == QDialog::Accepted) {
        if (newDialog.isGuideCreated) {
            addGuide(newDialog.newGuide);
        }
    }
}

/*************************************************************************/

void OptionsTab::on_pushButtonGuideImport_clicked(bool) {
    // Ask for filename
    QFileDialog dialog(this);
    dialog.setDirectory(curGuidesDialogPath);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("*.ttg");
    dialog.setDefaultSuffix("ttg");
    dialog.setViewMode(QFileDialog::Detail);

    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    }
    if (fileNames.isEmpty()) {
        return;
    }
    QString fileName = fileNames[0];
    curGuidesDialogPath = dialog.directory().absolutePath();
    QFile loadFile(fileName);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        MainWindow::displayMessage("Unable to open file!");
        return;
    }
    QJsonDocument loadDoc(QJsonDocument::fromJson(loadFile.readAll()));

    // Parse in data
    QJsonObject json = loadDoc.object();
    QString name = json["name"].toString();
    TiaSound::TvStandard standard = (json["TvStandard"] == "PAL" ? TiaSound::TvStandard::PAL : TiaSound::TvStandard::NTSC);
    double baseFreq = json["baseFrequency"].toDouble();
    TiaSound::PitchGuideFactory pgFactory;
    TiaSound::PitchGuide pg = pgFactory.calculateGuide(name, standard, baseFreq);

    addGuide(pg);
}

/*************************************************************************/

void OptionsTab::on_pushButtonGuideExport_clicked(bool) {
    QComboBox *cbGuides = findChild<QComboBox *>("comboBoxPitchGuide");
    TiaSound::PitchGuide *pg = &(guides[cbGuides->currentIndex()]);

    // Ask for filename
    QFileDialog dialog(this);
    dialog.setDirectory(curGuidesDialogPath);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("*.ttg");
    dialog.setDefaultSuffix("ttg");
    dialog.setViewMode(QFileDialog::Detail);
    dialog.selectFile(pg->name);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    }
    if (fileNames.isEmpty()) {
        return;
    }
    QString fileName = fileNames[0];
    curGuidesDialogPath = dialog.directory().absolutePath();
    QFile saveFile(fileName);

    // Export guide
    if (!saveFile.open(QIODevice::WriteOnly)) {
        MainWindow::displayMessage("Unable to open file!");
        return;
    }
    QJsonObject insObject;
    pg->toJson(insObject);
    QJsonDocument saveDoc(insObject);
    saveFile.write(saveDoc.toJson());
    saveFile.close();
}

/*************************************************************************/

void OptionsTab::on_lineEditAuthor_textChanged(const QString newText) {
    pTrack->metaAuthor = newText;
}

/*************************************************************************/

void OptionsTab::on_lineEditSongName_textChanged(const QString newText) {
    pTrack->metaName = newText;
}

/*************************************************************************/

void OptionsTab::on_plainTextEditComment_textChanged() {
    QPlainTextEdit *te = findChild<QPlainTextEdit *>("plainTextEditComment");
    pTrack->metaComment = te->document()->toPlainText();
}
