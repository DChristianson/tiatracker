#include "pianokeyboard.h"
#include <QPainter>

// Fixed key traits (black yes/no, position index)
const PianoKeyboard::keyGfxTrait PianoKeyboard::octaveTraits[] = {
    {false, 0},     // C
    {true, 1},      // Cis
    {false, 1},     // D
    {true, 2},      // Dis
    {false, 2},     // E
    {false, 3},     // F
    {true, 4},      // Fis
    {false, 4},     // G
    {true, 5},      // Gis
    {false, 5},     // A
    {true, 6},      // Ais
    {false, 6}      // H
};



PianoKeyboard::PianoKeyboard(QWidget *parent) : QWidget(parent)
{
    for (int i = 0; i < numKeys; ++i) {
        isPressed << false;
    }
    keyFont.setPixelSize(keyFontSize);
    QFontMetrics fontMetrics(keyFont);
    keyFontHeight = fontMetrics.height();

    setFixedWidth(keyboardWidth);
    setFixedHeight(keyboardHeight);
}



void PianoKeyboard::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::black);
    // White keys
    for (int octave = 0; octave < numOctaves; ++octave) {
        for (int i  = 0; i < numKeysPerOctave; ++i) {
            if (!octaveTraits[i].isBlack) {
                const int xPos = octave*keyWidth*numWhiteKeysPerOctave + keyWidth*octaveTraits[i].posIndex;
                painter.fillRect(xPos, 0, keyWidth, keyHeight, Qt::white);
                painter.drawRect(xPos, 0, keyWidth, keyHeight);
            }
        }
    }
    // Black keys
    for (int octave = 0; octave < numOctaves; ++octave) {
        for (int i  = 0; i < numKeysPerOctave; ++i) {
            if (octaveTraits[i].isBlack) {
                const int xPos = octave*keyWidth*numWhiteKeysPerOctave + keyWidth*octaveTraits[i].posIndex - blackKeyWidth/2;
                painter.fillRect(xPos, 0, blackKeyWidth, blackKeyHeight, Qt::black);
            }
        }
    }

    // Hints
    painter.setFont(keyFont);
    painter.setPen(Qt::white);
    painter.drawText(keyWidth - blackKeyWidth/2, 0, blackKeyWidth, blackKeyHeight, Qt::AlignBottom|Qt::AlignHCenter, "-37");

}
