#ifndef UNDOSTEP_H
#define UNDOSTEP_H

#include <QObject>

struct commandInfo {
    int selectedChannel = -1;
    int editPosFrom = -1;
    int editPosTo = -1;
    bool trackStats = false;
    bool trackTab = false;
    bool optionsTab = false;
    bool instrumentTab = false;
    bool percussionTab = false;
};

class UndoStep : public QObject
{
    Q_OBJECT

public:
    UndoStep(QObject* parent) :QObject(parent) {}

signals:
    void step(bool undo, const QString& text, const commandInfo& ci);
};

#endif // UNDOSTEP_H
