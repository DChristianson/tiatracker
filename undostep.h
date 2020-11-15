#ifndef UNDOSTEP_H
#define UNDOSTEP_H

#include <QObject>

struct commandInfo {
    int tab = -1;
    int selectedChannel = -1;
    int editPosFrom = -1;
    int editPosTo = -1;
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
