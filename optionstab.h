#ifndef OPTIONSTAB_H
#define OPTIONSTAB_H

#include <QObject>
#include <QWidget>
#include "track/track.h"
#include "emulation/player.h"
#include "tiasound/pitchguide.h"
#include "tiasound/pitchguidefactory.h"
#include <QList>


class OptionsTab : public QWidget
{
    Q_OBJECT
public:
    explicit OptionsTab(QWidget *parent = 0);

    void registerTrack(Track::Track *newTrack);

    /* Initializes the GUI components. Must be called once during init. */
    void initOptionsTab();

    /* Fills GUI elements with data from the track. Called upon changes. */
    void updateOptionsTab();

    QString curGuidesDialogPath;
    QList<TiaSound::PitchGuide> guides{};

signals:
    void setTVStandard(int);
    void setPitchGuide(TiaSound::PitchGuide *newGuide);

public slots:
    void on_comboBoxPitchGuide_currentIndexChanged(int index);

private:

    Track::Track *pTrack = nullptr;

    void addGuide(TiaSound::PitchGuide newGuide);

    bool bStartStringEditing = false;
    int iStartStringEditCount;

private slots:

    void on_radioButtonPal_toggled(bool checked);

    void on_pushButtonGuideCreate_clicked(bool);

    void on_pushButtonGuideImport_clicked(bool);

    void on_pushButtonGuideExport_clicked(bool);

    void on_lineEditAuthor_textChanged(const QString newText);
    void on_lineEditSongName_textChanged(const QString newText);
    void on_plainTextEditComment_textChanged();
    void on_text_editingFinished();

};

#endif // OPTIONSTAB_H
