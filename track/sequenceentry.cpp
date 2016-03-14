/* TIATracker, (c) 2016 Andre "Kylearan" Wichmann.
 * Website: https://bitbucket.org/kylearan/tiatracker
 * Email: andre.wichmann@gmx.de
 * See the file "license.txt" for information on usage and redistribution
 * of this file.
 */

#include "sequenceentry.h"

namespace Track {

SequenceEntry::SequenceEntry()
{

}

/*************************************************************************/

void SequenceEntry::toJson(QJsonObject &json) {
    json["patternindex"] = patternIndex;
    json["gototarget"] = gotoTarget;
}

/*************************************************************************/

bool SequenceEntry::fromJson(const QJsonObject &json) {
    patternIndex = json["patternindex"].toInt();
    gotoTarget = json["gototarget"].toInt();
    if (patternIndex < 0 || gotoTarget < -1) {
        return false;
    }
    return true;
}

}
