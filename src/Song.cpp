/*********************************************************************************/
/*!
@file           Song.cpp

@brief          xxxxx.

@author         L. J. Barman

    Copyright (c)   2008-2020, L. J. Barman and others, all rights reserved

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

#include "Song.h"
#include "Score.h"

#include <iostream>

void CSong::init2(CScore * scoreWin, CSettings* settings)
{

    CNote::reset();

    this->CConductor::init2(scoreWin, settings);

    setActiveHand(PB_PART_both);
    setPlayMode(PB_PLAY_MODE_followYou);
    setSpeed(1.0);
    setSkill(3);

    std::vector<int> startStopKeys;
    settings->getStartStopKeys(startStopKeys);

    std::vector<int> rewindKeys;
    settings->getReWindKeys(rewindKeys);

    std::vector<int> speedUpKeys;
    settings->getSpeedUpKeys(speedUpKeys);

    std::vector<int> speedDownKeys;
    settings->getSpeedDownKeys(speedDownKeys);

    auto iter = startStopKeys.begin();
    while ( iter != startStopKeys.end()) {
        specialKeySet.insert(*iter);
        startStopSet.insert(*iter);
        iter++;
    }

    iter = rewindKeys.begin();
    while ( iter != rewindKeys.end()) {
        specialKeySet.insert(*iter);
        restartSet.insert(*iter);
        iter++;
    }

    iter = speedUpKeys.begin();
    while ( iter != speedUpKeys.end()) {
        specialKeySet.insert(*iter);
        speedupSet.insert(*iter);
        iter++;
    }

    iter = speedDownKeys.begin();
    while ( iter != speedDownKeys.end()) {
        specialKeySet.insert(*iter);
        speeddownSet.insert(*iter);
        iter++;
    }

    speedDir = 0;

}

void CSong::loadSong(const QString & filename)
{
    CNote::reset();

    m_songTitle = filename;
    int index = m_songTitle.lastIndexOf("/");
    if (index >= 0)
        m_songTitle = m_songTitle.right( m_songTitle.length() - index - 1);

    QString fn = filename;
#ifdef _WIN32
     fn = fn.replace('/','\\');
#endif
    m_midiFile->setLogLevel(3);
    m_midiFile->openMidiFile(string(fn.toLocal8Bit().data()));
    ppLogInfo("Opening song %s",  fn.toLocal8Bit().data());
    transpose(0);
    midiFileInfo();
    m_midiFile->setLogLevel(99);
    playMusic(false);
    rewind();
    setPlayFromBar(0.0);
    setLoopingBars(0.0);
    setEventBits(EVENT_BITS_loadSong);
    if (!m_midiFile->getSongTitle().isEmpty())
        m_songTitle = m_midiFile->getSongTitle();

}

// read the file ahead to collect info about the song first
void CSong::midiFileInfo()
{
    m_trackList->reset(m_midiFile->numberOfTracks());
    setTimeSig(0,0);
    CStavePos::setKeySignature( NOT_USED, 0 );

    // Read the next events to find the active channels
    CMidiEvent event;
    while ( true )
    {
        event = m_midiFile->readMidiEvent();
        m_trackList->examineMidiEvent(event);

        if (event.type() == MIDI_PB_timeSignature)
        {
            setTimeSig(event.data1(),event.data2());
        }

        if (event.type() == MIDI_PB_EOF)
            break;
    }
}

void CSong::rewind()
{
    m_midiFile->rewind();
    this->CConductor::rewind();
    m_scoreWin->reset();
    reset();
    forceScoreRedraw();
}

void CSong::handleSpecialKey(CMidiEvent inputNote) {
    if ( specialKeySet.find(inputNote.note()) == specialKeySet.end()) {
        return;
    }

    if (inputNote.type() == MIDI_NOTE_ON) {
        specialNoteInputsOn.push_back(NoteInput(inputNote.note()));

        uint64_t curTime = timeSinceEpochMillisec2();

        vector<NoteInput>::iterator it = specialNoteInputsOn.begin();
        while ( it != specialNoteInputsOn.end()) {
            NoteInput val = *it;
            if ( curTime - val.time > 60 ) {
                it = specialNoteInputsOn.erase(it);
                continue;
            } else {
                it++;
            }
        }

        if (  specialNoteInputsOn.size() < 2 ) {    //Must map to more than 1 keys
            return;
        }

        std::unordered_set<int> startStopSet(this->startStopSet);
        std::unordered_set<int> restartSet(this->restartSet);
        std::unordered_set<int> speedupSet(this->speedupSet);
        std::unordered_set<int> speeddownSet(this->speeddownSet);

        for ( int i = specialNoteInputsOn.size() - 1; i >= 0; i-- ) {
            NoteInput val = specialNoteInputsOn.at(i);
            if ( restartSet.find(val.note) != restartSet.end()) {
                restartSet.erase(val.note);
            }
            if ( speedupSet.find(val.note) != speedupSet.end()) {
                speedupSet.erase(val.note);
            }
            if ( speeddownSet.find(val.note) != speedupSet.end()) {
                speeddownSet.erase(val.note);
            }
            if ( startStopSet.find(val.note) != startStopSet.end()) {
                startStopSet.erase(val.note);
            }
        }

        if ( restartSet.empty()) {
            specialNoteInputsOn.clear();
            playFromStartBar();
        }
        else if ( startStopSet.empty()) {
            playMusic(!playingMusic());
            songControlListener->playState(playingMusic());
            m_rating.addPlayedWrongNoteTally(-this->startStopSet.size());
            m_rating.addPlayedNoteTally(-this->startStopSet.size());
        }
        else if ( speedupSet.empty()) {
            //start the speeding up timer

            if ( speedDir == 1 ) {
                speedDir = 0;
                speedCtrlTime.stop();
            } else {
                speedDir = 1;
                if (! speedCtrlTime.isActive() ) {
                    speedCtrlTime.start(500, this);

                } else {
                    //speedDir = 0;
                    //speedCtrlTime.stop();
                }
            }
            //m_rating.addPlayedWrongNoteTally(-this->speedupSet.size());
            m_rating.addPlayedNoteTally(-this->speedupSet.size());
        } else if ( speeddownSet.empty()) {
            //start the speeding up timer
            if ( speedDir == -1 ) {
                speedDir = 0;
                speedCtrlTime.stop();
            } else {
                speedDir = -1;
                if (! speedCtrlTime.isActive() ) {
                    speedCtrlTime.start(500, this);
                } else {
                    //speedDir = 0;
                    //speedCtrlTime.stop();
                }
            }
            //m_rating.addPlayedWrongNoteTally(-this->speeddownSet.size());
            m_rating.addPlayedNoteTally(-this->speeddownSet.size());
        }
    }
}

void CSong::timerEvent(QTimerEvent *event) {
    float speed = this->getSpeed();
    speed = ((speed * 100.0) + (1 * speedDir))/100.0;
    this->setSpeed(speed);
    if ( songControlListener != NULL ) {
        songControlListener->speedChanged(speed);
    }
}

void CSong::setSongControlListener(SongControlListener * listener) {
    this->songControlListener = listener;
}

void CSong::pianistInput(CMidiEvent inputNote) {

    CConductor::pianistInput(inputNote);

    handleSpecialKey(inputNote);
}

void CSong::setActiveHand(whichPart_t hand)
{
    if (hand < PB_PART_both)
        hand = PB_PART_both;
    if (hand > PB_PART_left)
        hand = PB_PART_left;

    this->CConductor::setActiveHand(hand);
    regenerateChordQueue();

    m_scoreWin->setDisplayHand(hand);
}

void CSong::setActiveChannel(int chan)
{
    this->CConductor::setActiveChannel(chan);
    m_scoreWin->setActiveChannel(chan);
    regenerateChordQueue();
}

void  CSong::setPlayMode(playMode_t mode)
{
    regenerateChordQueue();
    this->CConductor::setPlayMode(mode);
    forceScoreRedraw();
}

void CSong::regenerateChordQueue()
{
    int i;
    int length;
    CMidiEvent event;

    m_wantedChordQueue->clear();
    m_findChord.reset();

    length = m_songEventQueue->length();

    for (i = 0; i < length; i++)
    {
        event = m_songEventQueue->index(i);
        // Find the next chord
        if (m_findChord.findChord(event, getActiveChannel(), PB_PART_both) == true)
            chordEventInsert( m_findChord.getChord() ); // give the Conductor the chord event

    }
    resetWantedChord();
}

void CSong::refreshScroll()
{
    m_scoreWin->refreshScroll();
    forceScoreRedraw();
}

eventBits_t CSong::task(int ticks)
{
    realTimeEngine(ticks);

    while (true)
    {
        if (m_reachedMidiEof == true)
            goto exitTask;

        while (true)
        {
            // Check that there is space
            if (midiEventSpace() <= 10 || chordEventSpace() <= 10)
                break;

            // and that the Score has space also
            if (m_scoreWin->midiEventSpace() <= 100)
                break;

            // Read the next events
            CMidiEvent event = m_midiFile->readMidiEvent();

            //ppLogTrace("Song event delta %d type 0x%x chan %d Note %d", event.deltaTime(), event.type(), event.channel(), event.note());

            // Find the next chord
            if (m_findChord.findChord(event, getActiveChannel(), PB_PART_both) == true)
                chordEventInsert( m_findChord.getChord() ); // give the Conductor the chord event

            // send the events to the other end
            m_scoreWin->midiEventInsert(event);

            // send the events to the other end
            midiEventInsert(event);

            if (event.type() == MIDI_PB_EOF)
            {
                m_reachedMidiEof = true;
                break;
            }
        }

        // carry on with the data until we reach the bar we want
        if (seekingBarNumber() && m_reachedMidiEof == false && playingMusic())
        {
            realTimeEngine(0);
            m_scoreWin->drawScrollingSymbols(false); // don't display any thing just  remove from the queue
        }
        else
            break;
    }

exitTask:
    eventBits_t eventBits = m_realTimeEventBits;
    m_realTimeEventBits = 0;
    return eventBits;
}

static const struct pcNote_s
{
    int key;
    int note;
} pcNoteLookup[] =
{
    { 'a', PC_KEY_LOWEST_NOTE },
    { 'z', 59 }, // B
    { 'x', 60 }, // Middle C
    { 'd', 61 },
    { 'c', 62 }, // D
    { 'f', 63 },
    { 'v', 64 }, // E
    { 'b', 65 }, // F
    { 'h', 66 },
    { 'n', 67 }, // G
    { 'j', 68 },
    { 'm', 69 }, // A
    { 'k', 70 },
    { ',', 71 }, // B
    { '.', 72 }, // C
    { ';', 73 },
    { '/', 74 }, // D
    { '\'', PC_KEY_HIGHEST_NOTE },
};

// Fakes a midi piano keyboard using the PC keyboard
bool CSong::pcKeyPress(int key, bool down)
{
    int i;
    size_t j;
    CMidiEvent midi;
    const int cfg_pcKeyVolume = 64;
    const int cfg_pcKeyChannel = 1-1;

    if (key == 't') // the tab key on the PC fakes good notes
    {
        if (down)
            m_fakeChord = getWantedChord();
        for (i = 0; i < m_fakeChord.length(); i++)
        {
            if (down)
                midi.noteOnEvent(0, cfg_pcKeyChannel, m_fakeChord.getNote(i).pitch() + getTranspose(), cfg_pcKeyVolume);
            else
                midi.noteOffEvent(0, cfg_pcKeyChannel, m_fakeChord.getNote(i).pitch() + getTranspose(), cfg_pcKeyVolume);
            expandPianistInput(midi);
        }
        return true;
    }

    for (j = 0; j < arraySize(pcNoteLookup); j++)
    {
        if ( key==pcNoteLookup[j].key)
        {
            if (down)
                midi.noteOnEvent(0, cfg_pcKeyChannel, pcNoteLookup[j].note, cfg_pcKeyVolume);
            else
                midi.noteOffEvent(0, cfg_pcKeyChannel, pcNoteLookup[j].note, cfg_pcKeyVolume);

            expandPianistInput(midi);
            return true;
        }
    }
    //printf("pcKeyPress %d %d\n", m_pcNote, key);
    return false;
}

