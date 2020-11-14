/* TIATracker, (c) 2016 Andre "Kylearan" Wichmann.
 * Website: https://bitbucket.org/kylearan/tiatracker
 * Email: andre.wichmann@gmx.de
 * See the file "license.txt" for information on usage and redistribution
 * of this file.
 */

#ifndef WAVEFORMSHAPER_H
#define WAVEFORMSHAPER_H

#include <QObject>
#include <QWidget>
#include <QList>
#include <QMenu>
#include "track/track.h"
#include "tiasound/tiasound.h"

class WaveformShaper : public QWidget
{
    Q_OBJECT
public:
    explicit WaveformShaper(QWidget *parent = 0);

    /* Register the instrument to modify */
    void registerPercussion(Track::Percussion *newPercussion);

    /* Set fixed size for layout according to envelope length */
    void updateSize();

    void setValues(QList<TiaSound::Distortion> newValues);

signals:
    void valuesChanged(const QList<TiaSound::Distortion>&);

public slots:
    void setWaveform(QAction *action);

protected:
    static const int legendCellSize = 20;
    static const int cellWidth = 16;

    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *) Q_DECL_OVERRIDE;

private:
    int calcWidth();

    // The percussion to edit
    Track::Percussion *pPercussion = nullptr;

    QList<TiaSound::Distortion> values;

    static const int valueFontSize = 11;
    static const int valueAreaMargin = 2;
    QFont valueFont{"Helvetica"};
    int valueFontHeight;
    int valueAreaHeight;

    QMenu contextMenu{this};
    int waveformColumn;
    TiaSound::Distortion distortionPen = TiaSound::Distortion::ELECTRONIC_RUMBLE;
};

#endif // WAVEFORMSHAPER_H
