/*********************************************************************************/
/*!
@file           Rating.h

@brief          The generates a score on how well the pianist is doing.

@author         L. J. Barman

    Copyright (c)   2008-2013, L. J. Barman, all rights reserved

    This file is part of the PianoBooster application

    PianoBooster is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PianoBooster is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PianoBooster.  If not, see <http://www.gnu.org/licenses/>.

*/
/*********************************************************************************/

#ifndef __RATING_H__
#define __RATING_H__

#include <iostream>

#include "Util.h"
#include "Cfg.h"

class CRating
{
public:
    CRating()
    {
        reset();
    }

    void reset();
    void countTotalNotes(int count) {
        m_totalNotesCount += count;
        //std::cout << "total: " << m_totalNotesCount << endl;
    }
    void countWrongNotes(int count) {
        m_wrongNoteCount += count;
        //std::cout << "wrong: " << m_wrongNoteCount << endl;
    }
    void countLateNotes(int count) {
        m_lateNoteCount += count;
        //std::cout << "late: " << m_lateNoteCount << endl;
    }
    int getTotalNoteCount() {return m_totalNotesCount;}
    int getWrongNoteCount() {return m_wrongNoteCount;}
    int getLateNoteCount() {return m_lateNoteCount;}

    double rating()
    {
        double percent = 100;
        if (m_totalNotesCount > 0)
            percent = ((m_totalNotesCount - m_lateNoteCount) * 100.0) /m_totalNotesCount;
        return percent;
    }

    void calculateAccuracy();

    float getAccuracyValue(){ return m_currentAccuracy; }
    CColor getAccuracyColor() { return m_currentColor; }
    bool isAccuracyGood() { return m_goodAccuracyFlag; }

    int getPlayedNoteTally() const;
    void addPlayedNoteTally(int value);

    int getPlayedWrongNoteTally() const;
    void addPlayedWrongNoteTally(int value);

    int getPlayedLateNoteTally() const;
    void addPlayedLateNoteTally(int value);

    int getTotalLateGood() const;

private:
    int m_totalNotesCount;
    int m_previousNoteCount;
    int m_lateNoteCount;
    int m_previousLateNoteCount;
    int m_wrongNoteCount;
    float m_currentAccuracy;
    float m_factor;
    CColor m_currentColor;
    bool m_goodAccuracyFlag;

    // Declaring new fields to avoid having conflict with existing fields
    int playedNoteTally;  //Total played notes up to that point in time
    int playedWrongNoteTally; //Total played notes being played wrong,
    int playedLateNoteTally;  //number of played notes that are played late up to the current time.
    int totalLateGood;  //number of notes that are late or good
};

#endif //__RATING_H__
