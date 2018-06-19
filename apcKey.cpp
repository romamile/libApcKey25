#include "apcKey.h"

apcKey::apcKey() {

}

void apcKey::setup() {
	state = false;

	std::cout << std::endl << " === Control Midi === " << std::endl;

	unsigned int nbPorts = 0;
	std::string portName;

	// RtMidiIn and Rt MidiOut constructor
	try { midiin = new RtMidiIn();   }
	catch (RtMidiError &error ) { printf("apcKey.constructor_er1\n"); error.printMessage(); exit( EXIT_FAILURE ); }
	try { midiout = new RtMidiOut(); }
	catch (RtMidiError &error ) { printf("apcKey.constructor_er2\n");  error.printMessage(); exit( EXIT_FAILURE ); }


	std::string midiName = "APC Key 25";

    std::cout << std::endl << " Searching for: " << midiName << std::endl;

	// Check inputs.
	nbPorts = midiin->getPortCount();
	std::cout << nbPorts << " MIDI input sources available." << std::endl;
	for (unsigned int i = 0; i<nbPorts; i++) {
		try { portName = midiin->getPortName(i); }
		catch (RtMidiError &error) { error.printMessage(); exit(EXIT_FAILURE); }
		std::cout << "-Input Port #" << i << ": " << portName << std::endl;

		if (portName.find(midiName) != std::string::npos) {
			state = true;
			midiin->openPort(i);
			std::cout << "MIDI IN :) OK!" << std::endl;
		}

	}

	// Check outputs.
	nbPorts = midiout->getPortCount();
	std::cout << nbPorts << " MIDI output ports available.\n";
	for (unsigned int i = 0; i<nbPorts; i++) {
		try { portName = midiout->getPortName(i); }
		catch (RtMidiError &error) { error.printMessage(); exit(EXIT_FAILURE); }
		std::cout << "-Output Port #" << i << ": " << portName << std::endl;

		if (portName.find(midiName) != std::string::npos) {
			state = true;
			midiout->openPort(i);
			std::cout << "MIDI OUT :) OK!" << std::endl;
		}

	}

	if (state == false)
		std::cout << "NO MIDI IN AND OUT :(" << std::endl;
	//	midiin->ignoreTypes(false, false, false);	

    // Initialisation
    reset();

}

void apcKey::reset() {

    for(int i = 0; i < 40; ++i) {
        sendMidi(messApcKey(akObj::CLIP, i, 0));
        clip[i] = 0;
        clipPressed[i] = false;
    }

    for(int i = 0; i < 8; ++i) {
        sendMidi(messApcKey(akObj::STOP, i, 0));
        stop[i] = 0;
        stopPressed[i] = false;
    }

    for(int i = 0; i < 5; ++i) {
        sendMidi(messApcKey(akObj::SCENE, i, 0));
        scene[i] = 0;
        scenePressed[i] = false;
    }


    stopAll = sustain = play = rec = shift = false;

}


apcKey::~apcKey() {
    for(int i = 0; i < 40; ++i)
        sendMidi(messApcKey(akObj::CLIP, i, 0));

    for(int i = 0; i < 8; ++i)
        sendMidi(messApcKey(akObj::STOP, i, 0));

    for(int i = 0; i < 5; ++i) 
        sendMidi(messApcKey(akObj::SCENE, i, 0));

	delete midiin; 
	delete midiout; 
}

void apcKey::pollMidi() {

	std::vector<unsigned char> m;
	m.resize(3);

	while( midiin->getMessage( &m ) ) { //Hmmm, plantouille, car le premier message à toujours "td == 0"

        //std::cout << "_______________" << std::endl;
        //for(int i = 0; i < m.size(); ++i)
        //    std::cout << (int)m[i] << std::endl;

        if(m[0] == 177 && m[1] == 64) {
            // SUSTAIN, chelou...
            listMess.push_back( messApcKey(akObj::SUSTAIN, 0, m[2] == 127 ? 1 : 0));
            sustain = m[2] == 127;

        } else if( 0 <= m[1] && m[1] <= 39 ) {
            // CLIPS
            listMess.push_back( messApcKey(akObj::CLIP, (4-m[1]/8)*8 + m[1]%8, m[0] == 144 ? 1 : 0));
            clipPressed[ (4-m[1]/8)*8 + m[1]%8 ] = m[0] == 144;

        } else if( 82 <= m[1] && m[1] <= 86 ) {
            // SCENE
            listMess.push_back( messApcKey(akObj::SCENE, m[1]-82, m[0] == 144 ? 1 : 0));
            scene[ m[1]-82 ] = m[0] == 144;

        } else if( 64 <= m[1] && m[1] <= 71 ) {
            // STOP
            listMess.push_back( messApcKey(akObj::STOP, m[1]-64, m[0] == 144 ? 1 : 0));
            stop[ m[1]-64 ] = m[0] == 144;

        } else if( 48 <= m[1] && m[1] <= 55 ) {
            // KNOBS
            listMess.push_back( messApcKey(akObj::KNOB, m[1]-48, m[2]));
            knob[ m[1]-48 ] = m[2];

        } else if( m[0] == 145 || m[0] == 129 ) {
            // KEYS
            listMess.push_back( messApcKey(akObj::KEY, m[1], m[0] == 145 ? m[2] : 0));

        } else {
            
            switch( m[1] ) {
            case 81: // STOP ALL
                listMess.push_back( messApcKey(akObj::STOP_ALL, 0, m[0] == 144 ? m[2] : 0));
                stopAll = m[0] == 144;
                break;

            case 91: // PLAY
                listMess.push_back( messApcKey(akObj::PLAY, 0, m[0] == 144 ? m[2] : 0));
                play = m[0] == 144;
                break;

            case 92: // REC
                listMess.push_back( messApcKey(akObj::REC, 0, m[0] == 144 ? m[2] : 0));
                rec = m[0] == 144;
                break;

            case 98: // SHIFT
                listMess.push_back( messApcKey(akObj::SHIFT, 0, m[0] == 144 ? m[2] : 0));
                shift = m[0] == 144;
                break;

            }
            

        }

	 }

}

void apcKey::sendMidi(messApcKey _mess) {

    // (0, 0 -> 39, 0 -> 6 ) // Black, green, green blink, red, red blink, yellow, yellow blink.

    if(!state) return;
    
    std::vector<unsigned char> midiMess;
    midiMess.resize(3);
    
    switch(_mess.obj) {
    case akObj::CLIP: // Blank, green, green blink, red, red blink, yellow, yellow blink.
        midiMess[0] = 144;
        midiMess[1] = (4-_mess.pos/8)*8 + _mess.pos%8;
        midiMess[2] = _mess.val;
        midiout->sendMessage( &midiMess );
        break;

    case akObj::STOP: // Blank, red, red blink
        midiMess[0] = 144;
        midiMess[1] = 64 + _mess.pos;
        midiMess[2] = _mess.val;
        midiout->sendMessage( &midiMess );
        break;

    case akObj::SCENE: // Blank, green, green blink
        midiMess[0] = 144;
        midiMess[1] = 82 + _mess.pos;
        midiMess[2] = _mess.val;
        midiout->sendMessage( &midiMess );
        break;

    }

    //std::cout << "sending midi: " << midiMess[0] << ", "  << midiMess[1] << ", "  << midiMess[2] << std::endl; 

}


