/* TIATracker, (c) 2016 Andre "Kylearan" Wichmann.
 * Website: https://bitbucket.org/kylearan/tiatracker
 * Email: andre.wichmann@gmx.de
 * See the file "license.txt" for information on usage and redistribution
 * of this file.
 */

#include "pattern.h"
#include <QJsonArray>
#include <mainwindow.h>


namespace Track {

Pattern::Pattern()
{

}

/*************************************************************************/

void Pattern::toJson(QJsonObject &json) {
    json["name"] = name;
    json["evenspeed"] = evenSpeed;
    json["oddspeed"] = oddSpeed;
    QJsonArray noteArray;
    for (int i = 0; i < notes.size(); ++i) {
        QJsonObject noteJson;
        notes[i].toJson(noteJson);
        noteArray.append(noteJson);
    }
    json["notes"] = noteArray;
}

/*************************************************************************/

bool Pattern::fromJson(const QJsonObject &json) {
    name = json["name"].toString();
    if (json.contains("evenspeed")) {
        evenSpeed = json["evenspeed"].toInt();
        oddSpeed = json["oddspeed"].toInt();
    } else {
        evenSpeed = 3;
        oddSpeed = 3;
    }
    QJsonArray noteArray = json["notes"].toArray();
    notes.clear();
    for (int i = 0; i < noteArray.size(); ++i) {
        Note newNote;
        if (!newNote.fromJson(noteArray[i].toObject())) {
            MainWindow::displayMessage("A pattern has an invalid note: " + name);
            return false;
        }
        notes.append(newNote);
    }
    return true;
}

}
