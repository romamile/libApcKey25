#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <string>

#include "./RTMidi/RtMidi.h"

enum class akObj   {KEY, KNOB, CLIP, SCENE, STOP, STOP_ALL, SUSTAIN, PLAY, REC, SHIFT};
enum class akLight {OFF, GREEN, GREEN_BLIN, RED, RED_BLINK, ORANGE, ORANGE_BLINK};
enum class akButt  {PRESSED, RELEASED, MOVED};


struct messApcKey {
    akObj obj;
    int pos;
    int val;

    messApcKey(akObj _obj,int _pos, int _val) {
        obj = _obj;
        pos = _pos;
        val = _val;
    }

};


class apcKey {
	
    private:
            // == Midi
		RtMidiIn  *midiin;
		RtMidiOut *midiout;
		bool state;
        

	public:
            // == Messages
		std::deque<messApcKey> listMess;


            // == States
		int clip[8 * 5];
		int scene[5];
		int stop[8];

		int knob[8];

            // == Button state (being pressed or not)
		bool clipPressed[8 * 5];
		bool scenePressed[5];
		bool stopPressed[8];

		bool stopAll;
        bool sustain, play, rec, shift;



	public:
		apcKey();
		~apcKey();

		void setup();
		void reset();
        void pollMidi();
        void sendMidi(akObj _obj, int _pos, int _val) { sendMidi(messApcKey(_obj, _pos, _val)); }
        void sendMidi(messApcKey _mess);
		bool isConnected() { return state; };
};
