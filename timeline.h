#ifndef TIMELINE_H
#define TIMELINE_H

#include <QObject>
#include <QWidget>
#include "track/track.h"

class Timeline : public QWidget
{
    Q_OBJECT
public:
    explicit Timeline(QWidget *parent = 0);

    void registerTrack(Track::Track *newTrack);

    QSize sizeHint() const;

signals:
    void changeEditPos(int newPos);

public slots:
    void editPosChanged(int newPos);

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    static const int channelWidth = 80;
    static const int channelMargin = 8;
    static const int channelGap = 8;
    static const int minHeight = 400;

    /* Calc row height in pixels */
    double calcRowHeight();

    int widgetWidth;
    int editPos = 0;

    Track::Track *pTrack = nullptr;

};

#endif // TIMELINE_H
