#include "Audio.hpp"

namespace SAGE {

	VirtualChannelUnit VirtualChannelUnit::operator+(const VirtualChannelUnit& oth) {
		VirtualChannelUnit toReturn;
		for (size_t i = 0; i < values.size(); i++) {
			toReturn.values[i] = values[i] + oth.values[i];
		}
		return toReturn;
	}

	float MixerDirectInput::runPlayback(size_t channel) {
		if (isPaused) return 0.f;
		size_t fudge = (size_t)playbackCursor;
		if (channel == channelBuffers.size() - 1) {
			playbackCursor = playbackCursor + (playbackFrequency * playbackSpeed);
			while (playbackCursor > loopEnd) playbackCursor = loopBeg + (playbackCursor - loopEnd);
			while (playbackCursor < loopBeg) playbackCursor = loopEnd - (loopBeg - playbackCursor);
		}

		if (isMuted) return 0.f;

		return channelBuffers[channel][fudge] * playbackVolume;
	}


	Synthesizer::ADSREnvelope::ADSREnvelope() {//rest
		buildup = 0.f;
		attackTime = 0.f;
		fall = 0.f;
		decayTime = 0.f;
		stay = 0.f;
		sustainTime = 0.f;
		dropoff = 0.f;
		releaseTime = 0.f;
		maxVol = 0.f;
		susVol = 0.f;
		lifetime = 0.0;
		drone = true;
		playing = false;
	}
	Synthesizer::ADSREnvelope::ADSREnvelope(float a, float d) {//percussion/pluck string
		buildup = a;
		attackTime = a;
		fall = d;
		decayTime = a + d;
		stay = 0.f;
		sustainTime = a + d;
		dropoff = 0.f;
		releaseTime = a + d;
		maxVol = 1.f;
		susVol = 0.f;
		lifetime = 0.0;
		drone = false;
		playing = false;
	}
	Synthesizer::ADSREnvelope::ADSREnvelope(float a, float d, float s, float r, bool cont) {//wind/drawn string
		buildup = a;
		attackTime = buildup;
		fall = d;
		decayTime = attackTime + fall;
		stay = s;
		sustainTime = decayTime + stay;
		dropoff = r;
		releaseTime = sustainTime + dropoff;
		maxVol = 1.f;
		susVol = 0.5f;
		lifetime = 0.0;
		drone = cont;
		playing = false;
	}
	double Synthesizer::ADSREnvelope::getAmplitude(double dTime) {
		double fAmp = 0.0;
		//Attack
		if (lifetime <= attackTime) {
			fAmp = (lifetime / attackTime) * maxVol;
		}
		//Decay
		else if (lifetime <= decayTime) {
			fAmp = maxVol - (((lifetime - attackTime) / fall) * (maxVol - susVol));
		}
		//Sustain
		else if (lifetime <= sustainTime) {
			fAmp = susVol;
		}
		//R
		else if (lifetime <= releaseTime) {
			fAmp = susVol - (((lifetime - sustainTime) / dropoff) * (susVol));
		}
		lifetime += dTime;
		return fAmp < maxVol ? fAmp : maxVol;
	}
	Synthesizer::Note::Note() {
		waveTable = nullptr;
		envelope = ADSREnvelope();
		frequency = 0.f;
		lifetime = 0;
	}
	double Synthesizer::Note::sound(double dTime) {
		unsigned int tabledex = (int)(lifetime++ * frequency) % tableSize;
		if (lifetime > tableSize) lifetime -= tableSize;
		return envelope.getAmplitude(dTime) * waveTable[tabledex];
	}
	Synthesizer::Instrument::Instrument() {
		baseFrequency = 0;
		attackDuration = 0;
		decayDuration = 0;
		releaseDuration = 0;
	}
	Synthesizer::PlaybackInstrument::PlaybackInstrument(External_WaveFileData& info):
	Instrument(){
		timbre.resize(info.dataSize);
	}
	void Synthesizer::PlaybackInstrument::generateTimbre() {}
	Synthesizer::Note Synthesizer::PlaybackInstrument::play(short freq, double time) {
		Note note{};
		note.envelope = ADSREnvelope(attackDuration, decayDuration, time, releaseDuration, true);//time comes from file...
		note.waveTable = timbre.data();
		note.tableSize = timbre.size();
		note.frequency = 1;
		note.lifetime = 0.0;
		return note;
	}
	Synthesizer::SinInstrument::SinInstrument():
	Instrument(){
		generateTimbre();
		attackDuration = 0.05f;
		decayDuration = 0.2f;
		releaseDuration = 0.5f;
	}
	void Synthesizer::SinInstrument::generateTimbre() {
		timbre.resize(hardwareInfo.soundCardInfo.outputHertz);
		double phase = 0.0;
		for (int i = 0; i < timbre.size(); i++) {
			timbre[i] = oscSin(1.f, phase);
			phase += 1.0 / hardwareInfo.soundCardInfo.outputHertz;
		}
	}
	Synthesizer::Note Synthesizer::SinInstrument::play(short n, double d){
		Note note{};
		note.envelope = ADSREnvelope(attackDuration, decayDuration, d, releaseDuration, false);
		note.waveTable = timbre.data();
		note.tableSize = timbre.size();
		note.frequency = n;
		note.lifetime = 0.0;
		return note;
	}

	void convertOutputUint8(std::vector<float>& in, Uint8* out){
		Uint8* sample = (Uint8*)out;
		for (size_t i = 0; i < in.size(); i++) {
			sample[i] = (in[i] + 1.f) * CHAR_MAX;
		}
	}
	void convertOutputSint8(std::vector<float>& in, Uint8* out) {
		Sint8* sample = (Sint8*)out;
		for (size_t i = 0; i < in.size(); i++) {
			sample[i] = in[i] * CHAR_MAX;
		}
	}
	void convertOutputUint16(std::vector<float>& in, Uint8* out) {
		Uint16* sample = (Uint16*)out;
		for (size_t i = 0; i < in.size(); i++) {
			sample[i] = (in[i] + 1.f) * INT16_MAX;
		}
	}
	void convertOutputSint16(std::vector<float>& in, Uint8* out) {
		Sint16* sample = (Sint16*)out;
		for (size_t i = 0; i < in.size(); i++) {
			sample[i] = in[i] * INT16_MAX;
		}
	}
	void convertOutputUint32(std::vector<float>& in, Uint8* out) {
		Uint32* sample = (Uint32*)out;
		for (size_t i = 0; i < in.size(); i++) {
			sample[i] = (in[i] + 1.f) * INT32_MAX;
		}
	}
	void convertOutputSint32(std::vector<float>& in, Uint8* out) {
		Sint32* sample = (Sint32*)out;
		for (size_t i = 0; i < in.size(); i++) {
			sample[i] = in[i] * INT32_MAX;
		}
	}
	void convertOutputUint64(std::vector<float>& in, Uint8* out) {
		Uint64* sample = (Uint64*)out;
		for (size_t i = 0; i < in.size(); i++) {
			sample[i] = (in[i] + 1.f) * INT64_MAX;
		}
	}
	void convertOutputSint64(std::vector<float>& in, Uint8* out) {
		Sint64* sample = (Sint64*)out;
		for (size_t i = 0; i < in.size(); i++) {
			sample[i] = in[i] * INT64_MAX;
		}
	}
	void convertOutputFloat32(std::vector<float>& in, Uint8* out){
		float* sample = (float*)out;
		for (size_t i = 0; i < in.size(); i++) {
			sample[i] = in[i];
		}
	}

	Mixer::Mixer() {
		outBuffer = std::vector<float>(hardwareInfo.soundCardInfo.outputSamplesPerChannel * hardwareInfo.soundCardInfo.outputNumChannels);
		channels = std::vector<AudioChannel>(hardwareInfo.soundCardInfo.outputNumChannels);
		for (auto& chan : channels) {
			chan = AudioChannel(hardwareInfo.soundCardInfo.outputSamplesPerChannel);
		}
		audioDeltaTime = 0.0;
		volume = 0.05f;//this is linear right now, but iirc dB is a logarithmic scale so...will revisit volume later four shore
		Uint16 bitsize = SDL_AUDIO_MASK_BITSIZE & hardwareInfo.soundCardInfo.outputBitDepth;
		if (SDL_AUDIO_ISFLOAT(hardwareInfo.soundCardInfo.outputBitDepth)) {
			conversionFunction = convertOutputFloat32;
		}
		else if (SDL_AUDIO_ISSIGNED(hardwareInfo.soundCardInfo.outputBitDepth)) {
			switch (bitsize) {
			case 8:
				conversionFunction = convertOutputSint8;
				break;
			case 16:
				conversionFunction = convertOutputSint16;
				break;
			case 32:
				conversionFunction = convertOutputSint32;
				break;
			case 64:
				conversionFunction = convertOutputSint64;
				break;
			}
		}
		else {
			switch (bitsize) {
			case 8:
				conversionFunction = convertOutputUint8;
				break;
			case 16:
				conversionFunction = convertOutputUint16;
				break;
			case 32:
				conversionFunction = convertOutputUint32;
				break;
			case 64:
				conversionFunction = convertOutputUint64;
				break;
			}
		}
	}
	void Mixer::recordFromSynth(Synthesizer::Note& n) {
		//ideally, both of the following functions are rolled into one
		//just take in a sound output component and some other methods decide how the sound gets into that format
		activeNotes.push_back(n);
	}
	void Mixer::recordFromFile(MixerDirectInput* f) {
		//do some preprocessing here to switch the channels around to fit with the mixer's channel layout
		//this will guarantee them to be simple to process
		precompSounds.push_back(f);
	}
	void Mixer::integrate(VirtualChannelUnit& in) {
		//we must guarantee that the input is the sum total of all virtual channel values in a given sample frame
		float workingVal = 0.f;
		switch (channels.size()) {
		case 0:
			std::cout << "What kind of sound device doesn't have any channels to output to?";
			break;
		case 1:
			for (size_t i = 0;i < in.values.size(); i++) {
				workingVal += in.values[i];
			}
			channels[0].write(workingVal * volume);
			break;
		case 2:
			//sL, sR
			workingVal =
				in.values[MID_FRONT_LEFT] * vspd(TRUE_LEFT, MID_FRONT_LEFT) +
				in.values[MID_FRONT_CENTER] * 0.5f +
				in.values[SUBWOOFER] * 0.5f +
				in.values[MID_BACK_LEFT] * vspd(TRUE_LEFT, MID_BACK_LEFT) +
				in.values[MID_FRONT_CENTER_LEFT] * vspd(TRUE_LEFT, MID_FRONT_CENTER_LEFT) +
				in.values[MID_BACK_CENTER] * 0.5f +
				in.values[TRUE_LEFT] +
				in.values[OVERHEAD] * 0.5f +
				in.values[UP_FRONT_LEFT] * vspd(TRUE_LEFT, UP_FRONT_LEFT) +
				in.values[UP_FRONT_CENTER] * 0.5f +
				in.values[UP_BACK_LEFT] * vspd(TRUE_LEFT, UP_BACK_LEFT) +
				in.values[UP_BACK_CENTER] * 0.5f;
			channels[0].write(workingVal * volume);

			workingVal =
				in.values[MID_FRONT_RIGHT] * vspd(TRUE_RIGHT, MID_FRONT_RIGHT) +
				in.values[MID_FRONT_CENTER] * 0.5f +
				in.values[SUBWOOFER] * 0.5f +
				in.values[MID_BACK_RIGHT] * vspd(TRUE_RIGHT, MID_BACK_RIGHT) +
				in.values[MID_FRONT_CENTER_RIGHT] * vspd(TRUE_RIGHT, MID_FRONT_CENTER_RIGHT) +
				in.values[MID_BACK_CENTER] * 0.5f +
				in.values[TRUE_RIGHT] +
				in.values[OVERHEAD] * 0.5f +
				in.values[UP_FRONT_RIGHT] * vspd(TRUE_RIGHT, UP_FRONT_RIGHT) +
				in.values[UP_FRONT_CENTER] * 0.5f +
				in.values[UP_BACK_RIGHT] * vspd(TRUE_RIGHT, UP_BACK_RIGHT) +
				in.values[UP_BACK_CENTER] * 0.5f;
			channels[1].write(workingVal * volume);
			break;
		case 3:
			//sL, sR, sW
			workingVal =
				in.values[MID_FRONT_LEFT] * vspd(TRUE_LEFT, MID_FRONT_LEFT) +
				in.values[MID_FRONT_CENTER] * 0.5f +
				in.values[MID_BACK_LEFT] * vspd(TRUE_LEFT, MID_BACK_LEFT) +
				in.values[MID_FRONT_CENTER_LEFT] * vspd(TRUE_LEFT, MID_FRONT_CENTER_LEFT) +
				in.values[MID_BACK_CENTER] * 0.5f +
				in.values[TRUE_LEFT]+
				in.values[OVERHEAD] * 0.5f +
				in.values[UP_FRONT_LEFT] * vspd(TRUE_LEFT, UP_FRONT_LEFT) +
				in.values[UP_FRONT_CENTER] * 0.5f +
				in.values[UP_BACK_LEFT] * vspd(TRUE_LEFT, UP_BACK_LEFT) +
				in.values[UP_BACK_CENTER] * 0.5f;
			channels[0].write(workingVal * volume);

			workingVal =
				in.values[MID_FRONT_RIGHT] * vspd(TRUE_RIGHT, MID_FRONT_RIGHT) +
				in.values[MID_FRONT_CENTER] * 0.5f +
				in.values[MID_BACK_RIGHT] * vspd(TRUE_RIGHT, MID_BACK_RIGHT) +
				in.values[MID_FRONT_CENTER_RIGHT] * vspd(TRUE_RIGHT, MID_FRONT_CENTER_RIGHT) +
				in.values[MID_BACK_CENTER] * 0.5f +
				in.values[TRUE_RIGHT] +
				in.values[OVERHEAD] * 0.5f +
				in.values[UP_FRONT_RIGHT] * vspd(TRUE_RIGHT, UP_FRONT_RIGHT) +
				in.values[UP_FRONT_CENTER] * 0.5f +
				in.values[UP_BACK_RIGHT] * vspd(TRUE_RIGHT, UP_BACK_RIGHT) +
				in.values[UP_BACK_CENTER] * 0.5f;
			channels[1].write(workingVal * volume);

			channels[2].write(in.values[SUBWOOFER] * volume);
			
			break;
		case 4:
			//fL, fR, bL, bR
			workingVal =
				in.values[MID_FRONT_LEFT] +
				in.values[MID_FRONT_CENTER] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER) +
				in.values[SUBWOOFER]* 0.25f +
				in.values[MID_FRONT_CENTER_LEFT] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER_LEFT) +
				in.values[TRUE_LEFT] * vspd(MID_FRONT_LEFT, TRUE_LEFT) +
				in.values[OVERHEAD]* 0.25f +
				in.values[UP_FRONT_LEFT] * vspd(MID_FRONT_LEFT, UP_FRONT_LEFT) +
				in.values[UP_FRONT_CENTER] * vspd(MID_FRONT_LEFT, UP_FRONT_CENTER);
			channels[0].write(workingVal * volume);


			workingVal =
				in.values[MID_FRONT_RIGHT] +
				in.values[MID_FRONT_CENTER] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER) +
				in.values[SUBWOOFER] * 0.25f +
				in.values[MID_FRONT_CENTER_RIGHT] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER_RIGHT) +
				in.values[TRUE_RIGHT] * vspd(MID_FRONT_RIGHT, TRUE_RIGHT) +
				in.values[OVERHEAD] * 0.25f +
				in.values[UP_FRONT_RIGHT] * vspd(MID_FRONT_RIGHT, UP_FRONT_RIGHT) +
				in.values[UP_FRONT_CENTER] * vspd(MID_FRONT_RIGHT, UP_FRONT_CENTER);
			channels[1].write(workingVal * volume);

			workingVal =
				in.values[SUBWOOFER] * 0.25f +
				in.values[MID_BACK_LEFT] +
				in.values[MID_BACK_CENTER] * vspd(MID_BACK_LEFT, MID_BACK_CENTER) +
				in.values[TRUE_LEFT] * vspd(MID_BACK_LEFT, TRUE_LEFT) +
				in.values[OVERHEAD] * 0.25f +
				in.values[UP_BACK_LEFT] * vspd(MID_BACK_LEFT, UP_BACK_LEFT) +
				in.values[UP_BACK_CENTER] * vspd(MID_BACK_LEFT, UP_BACK_CENTER);
			channels[2].write(workingVal * volume);

			workingVal =
				in.values[SUBWOOFER] * 0.25f +
				in.values[MID_BACK_RIGHT] +
				in.values[MID_BACK_CENTER] * vspd(MID_BACK_RIGHT, MID_BACK_CENTER) +
				in.values[TRUE_RIGHT] * vspd(MID_BACK_RIGHT, TRUE_RIGHT) +
				in.values[OVERHEAD] * 0.25f +
				in.values[UP_BACK_RIGHT] * vspd(MID_BACK_RIGHT, UP_BACK_RIGHT) +
				in.values[UP_BACK_CENTER] * vspd(MID_BACK_RIGHT, UP_BACK_CENTER);
			channels[3].write(workingVal * volume);
			break;
		case 5:
			//fL, fR, sW, bL, bR
			workingVal =
				in.values[MID_FRONT_LEFT] +
				in.values[MID_FRONT_CENTER] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER) +
				in.values[MID_FRONT_CENTER_LEFT] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER_LEFT) +
				in.values[TRUE_LEFT] * vspd(MID_FRONT_LEFT, TRUE_LEFT) +
				in.values[OVERHEAD] * 0.25f +
				in.values[UP_FRONT_LEFT] * vspd(MID_FRONT_LEFT, UP_FRONT_LEFT) +
				in.values[UP_FRONT_CENTER] * vspd(MID_FRONT_LEFT, UP_FRONT_CENTER);
			channels[0].write(workingVal * volume);

			workingVal =
				in.values[MID_FRONT_RIGHT] +
				in.values[MID_FRONT_CENTER] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER) +
				in.values[MID_FRONT_CENTER_RIGHT] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER_RIGHT) +
				in.values[TRUE_RIGHT] * vspd(MID_FRONT_RIGHT, TRUE_RIGHT) +
				in.values[OVERHEAD] * 0.25f +
				in.values[UP_FRONT_RIGHT] * vspd(MID_FRONT_RIGHT, UP_FRONT_RIGHT) +
				in.values[UP_FRONT_CENTER] * vspd(MID_FRONT_RIGHT, UP_FRONT_CENTER);
			channels[1].write(workingVal * volume);

			channels[2].write(in.values[SUBWOOFER] * volume);

			workingVal =
				in.values[MID_BACK_LEFT] +
				in.values[MID_BACK_CENTER] * vspd(MID_BACK_LEFT, MID_BACK_CENTER) +
				in.values[TRUE_LEFT] * vspd(MID_BACK_LEFT, TRUE_LEFT) +
				in.values[OVERHEAD] * 0.25f +
				in.values[UP_BACK_LEFT] * vspd(MID_BACK_LEFT, UP_BACK_LEFT) +
				in.values[UP_BACK_CENTER] * vspd(MID_BACK_LEFT, UP_BACK_CENTER);
			channels[3].write(workingVal * volume);

			workingVal =
				in.values[MID_BACK_RIGHT] +
				in.values[MID_BACK_CENTER] * vspd(MID_BACK_RIGHT, MID_BACK_CENTER) +
				in.values[TRUE_RIGHT] * vspd(MID_BACK_RIGHT, TRUE_RIGHT) +
				in.values[OVERHEAD] * 0.25f +
				in.values[UP_BACK_RIGHT] * vspd(MID_BACK_RIGHT, UP_BACK_RIGHT) +
				in.values[UP_BACK_CENTER] * vspd(MID_BACK_RIGHT, UP_BACK_CENTER);
			channels[4].write(workingVal * volume);
			break;
		case 6:
			//fL, fR, fC, sW, sL, sR
			workingVal =
				in.values[MID_FRONT_LEFT] +
				in.values[MID_FRONT_CENTER_LEFT] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER_LEFT) +
				in.values[OVERHEAD] * 0.2f +
				in.values[UP_FRONT_LEFT] * vspd(MID_FRONT_LEFT, UP_FRONT_LEFT);
			channels[0].write(workingVal * volume);

			workingVal =
				in.values[MID_FRONT_RIGHT] +
				in.values[MID_FRONT_CENTER_RIGHT] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER_RIGHT) +
				in.values[OVERHEAD] * 0.2f +
				in.values[UP_FRONT_RIGHT] * vspd(MID_FRONT_RIGHT, UP_FRONT_RIGHT);
			channels[1].write(workingVal * volume);

			workingVal =
				in.values[MID_FRONT_CENTER] +
				in.values[MID_FRONT_CENTER_LEFT] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_LEFT) +
				in.values[MID_FRONT_CENTER_RIGHT] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_RIGHT) +
				in.values[OVERHEAD] * 0.2f +
				in.values[UP_FRONT_CENTER] * vspd(MID_FRONT_CENTER, UP_FRONT_CENTER);
			channels[2].write(workingVal * volume);

			channels[3].write(in.values[SUBWOOFER] * volume);

			workingVal =
				in.values[MID_BACK_LEFT] * vspd(TRUE_LEFT, MID_BACK_LEFT) +
				in.values[MID_BACK_CENTER] * 0.5f +
				in.values[TRUE_LEFT] +
				in.values[OVERHEAD] * 0.2f +
				in.values[UP_BACK_LEFT] * vspd(TRUE_LEFT, UP_BACK_LEFT) +
				in.values[UP_BACK_CENTER] * 0.5f;
			channels[4].write(workingVal * volume);

			workingVal =
				in.values[MID_BACK_RIGHT] * vspd(TRUE_RIGHT, MID_BACK_RIGHT) +
				in.values[MID_BACK_CENTER] * 0.5f +
				in.values[TRUE_RIGHT] +
				in.values[OVERHEAD] * 0.2f +
				in.values[UP_BACK_RIGHT] * vspd(TRUE_RIGHT, UP_BACK_RIGHT) +
				in.values[UP_BACK_CENTER] * 0.5f;
			channels[5].write(workingVal * volume);
			
			break;
		case 7:
			//fL, fR, fC, sW, bC, sL, sR
			workingVal =
				in.values[MID_FRONT_LEFT] +
				in.values[MID_FRONT_CENTER_LEFT] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER_LEFT) +
				in.values[OVERHEAD] / 6.f +
				in.values[UP_FRONT_LEFT] * vspd(MID_FRONT_LEFT, UP_FRONT_LEFT);
			channels[0].write(workingVal * volume);

			workingVal =
				in.values[MID_FRONT_RIGHT] +
				in.values[MID_FRONT_CENTER_RIGHT] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER_RIGHT) +
				in.values[OVERHEAD] / 6.f +
				in.values[UP_FRONT_RIGHT] * vspd(MID_FRONT_RIGHT, UP_FRONT_RIGHT);
			channels[1].write(workingVal * volume);

			workingVal =
				in.values[MID_FRONT_CENTER] +
				in.values[MID_FRONT_CENTER_LEFT] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_LEFT) +
				in.values[MID_FRONT_CENTER_RIGHT] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_RIGHT) +
				in.values[OVERHEAD] / 6.f +
				in.values[UP_FRONT_CENTER] * vspd(MID_FRONT_CENTER, UP_FRONT_CENTER);
			channels[2].write(workingVal * volume);

			channels[3].write(in.values[SUBWOOFER] * volume);

			workingVal =
				in.values[MID_BACK_LEFT] * vspd(MID_BACK_CENTER, MID_BACK_LEFT) +
				in.values[MID_BACK_RIGHT] * vspd(MID_BACK_CENTER, MID_BACK_RIGHT) +
				in.values[MID_BACK_CENTER] +
				in.values[OVERHEAD] / 6.f +
				in.values[UP_BACK_LEFT] * vspd(MID_BACK_CENTER, UP_BACK_LEFT) +
				in.values[UP_BACK_CENTER] * vspd(MID_BACK_CENTER, UP_BACK_CENTER) +
				in.values[UP_BACK_RIGHT] * vspd(MID_BACK_CENTER, UP_BACK_RIGHT);
			channels[4].write(workingVal * volume);

			workingVal =
				in.values[MID_BACK_LEFT] * vspd(TRUE_LEFT, MID_BACK_LEFT) +
				in.values[TRUE_LEFT] +
				in.values[OVERHEAD] / 6.f +
				in.values[UP_BACK_LEFT] * vspd(TRUE_LEFT, UP_BACK_LEFT);
			channels[5].write(workingVal * volume);

			workingVal =
				in.values[MID_BACK_RIGHT] * vspd(TRUE_RIGHT, MID_BACK_RIGHT) +
				in.values[TRUE_RIGHT] +
				in.values[OVERHEAD] / 6.f +
				in.values[UP_BACK_RIGHT] * vspd(TRUE_RIGHT, UP_BACK_RIGHT);
			channels[6].write(workingVal * volume);
			
			break;
		case 8:
			//fL, fR, fC, sW, bL, bR, sL, sR
			workingVal =
				in.values[MID_FRONT_LEFT] +
				in.values[MID_FRONT_CENTER_LEFT] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER_LEFT) +
				in.values[OVERHEAD] / 7.f +
				in.values[UP_FRONT_LEFT] * vspd(MID_FRONT_LEFT, UP_FRONT_LEFT);
			channels[0].write(workingVal * volume);

			workingVal =
				in.values[MID_FRONT_RIGHT] +
				in.values[MID_FRONT_CENTER_RIGHT] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER_RIGHT) +
				in.values[OVERHEAD] / 7.f +
				in.values[UP_FRONT_RIGHT] * vspd(MID_FRONT_RIGHT, UP_FRONT_RIGHT);
			channels[1].write(workingVal * volume);

			workingVal =
				in.values[MID_FRONT_CENTER] +
				in.values[MID_FRONT_CENTER_LEFT] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_LEFT) +
				in.values[MID_FRONT_CENTER_RIGHT] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_RIGHT) +
				in.values[OVERHEAD] / 7.f +
				in.values[UP_FRONT_CENTER] * vspd(MID_FRONT_CENTER, UP_FRONT_CENTER);
			channels[2].write(workingVal * volume);

			channels[3].write(in.values[SUBWOOFER] * volume);

			workingVal =
				in.values[MID_BACK_LEFT] +
				in.values[MID_BACK_CENTER] * vspd(MID_BACK_LEFT, MID_BACK_CENTER) +
				in.values[OVERHEAD] / 7.f +
				in.values[UP_BACK_LEFT] * vspd(MID_BACK_LEFT, UP_BACK_LEFT) +
				in.values[UP_BACK_CENTER] * vspd(MID_BACK_LEFT, UP_BACK_CENTER);
			channels[4].write(workingVal * volume);

			workingVal =
				in.values[MID_BACK_RIGHT] +
				in.values[MID_BACK_CENTER] * vspd(MID_BACK_RIGHT, MID_BACK_CENTER) +
				in.values[OVERHEAD] / 7.f +
				in.values[UP_BACK_CENTER] * vspd(MID_BACK_RIGHT, UP_BACK_CENTER) +
				in.values[UP_BACK_RIGHT] * vspd(MID_BACK_RIGHT, UP_BACK_RIGHT);
			channels[5].write(workingVal * volume);

			workingVal =
				in.values[TRUE_LEFT] +
				in.values[OVERHEAD] / 7.f;
			channels[6].write(workingVal * volume);

			workingVal =
				in.values[TRUE_RIGHT] +
				in.values[OVERHEAD] / 7.f;
			channels[7].write(workingVal * volume);
			
			break;
		default:
			std::cout << "We don't support that many channels yet! (but I like your style)";
			for (AudioChannel& c : channels) {
				c.write(0.f);
			}
			break;
		}
	}
	void Mixer::update(double dTime) {
		audioDeltaTime += dTime;
		if (channelsFree()) {
			for (size_t i = 0; i < hardwareInfo.soundCardInfo.outputSamplesPerChannel; i++) {
				VirtualChannelUnit tots;
				double total = 0.0;
				for (auto& n : activeNotes) {
					total += n.sound(audioDeltaTime / hardwareInfo.soundCardInfo.outputSamplesPerChannel);
				}
				for (int j = 0; j < hardwareInfo.soundCardInfo.outputNumChannels; j++) {
					for (auto& s : precompSounds) {
						//add values into the total for that channel
						total += s->runPlayback(j);
					}
					channels[j].write(volume * total);//integrate will now contain the writes
				}
			}
			audioDeltaTime = 0.0;
		}
	}
	bool Mixer::channelsFree() {
		for (auto c : channels) {
			if (!c.canWrite(hardwareInfo.soundCardInfo.outputSamplesPerChannel))return false;
		}
		return true;
	}
	void Mixer::prepareOutput() {
		for (int i = 0; i < hardwareInfo.soundCardInfo.outputNumChannels; i++) {
			for (int j = 0; j < hardwareInfo.soundCardInfo.outputSamplesPerChannel; j++) {
				outBuffer[i + (j * hardwareInfo.soundCardInfo.outputNumChannels)] = channels[i].read();
				//this is where we want to convert to the value the driver wants
				//problem being...outbuffer would have a type that varies at runtime.
				//maybe conversion in the callback won't be a problem
				//but if it does present a problem...figure out what to do here.
			}
			channels[i].writeLock(false);
		}
	}
	void Mixer::outputSound(Uint8* stream) {
		for (auto& c : channels) {
			c.readLock(false);
		}
		conversionFunction(outBuffer, stream);//hello
		prepareOutput();
	}
	Mixer::AudioChannel::AudioChannel(int bufferSize) {
			buffer = std::vector<float>(bufferSize * 3);
			readHead = 0;
			writeHead = 0;
			readIdle = true;
			writeIdle = false;
		}
	float Mixer::AudioChannel::read() {
			if (readIdle) return buffer[readHead];
			if (readHead == writeHead) readIdle = true;
			float value = buffer[readHead];
			++readHead %= buffer.size();
			return value;
		}
	void Mixer::AudioChannel::write(float value) {
			if (writeIdle) return;
			if ((writeHead + 1) % buffer.size() == readHead) writeIdle = true;
			buffer[writeHead] = value;
			++writeHead %= buffer.size();
		}
	bool Mixer::AudioChannel::canWrite(int values) {
			if (writeIdle) return false;
			int hypo = writeHead + values;
			if (writeHead < readHead) {
				return hypo < readHead;
			}
			if (hypo > buffer.size()) {
				return hypo % buffer.size() < readHead;
			}
			return true;
		}

	Sequencer::Sequencer() {
		mixer = nullptr;
		elements = std::vector<SequencerElement>();
		bpm = 120;
		beatTime = 0.5;
		elapsedTime = 0.0;
		elapsedBeats = 0.0;
	}
	void Sequencer::add(Synthesizer::Instrument* i, int beatNum, float freq) {
		elements.push_back(SequencerElement(i, beatNum * beatTime, freq));
	}
	void Sequencer::update(double dTime) {
		elapsedTime += dTime;
		elapsedBeats = elapsedTime / beatTime;

		for (auto& se : elements) {
			if (elapsedBeats >= se.time) {
				//send to mixer (or synth class?) for processing and mark for removal
				mixer->recordFromSynth(se.note);
				se.active = false;
			}
		}
		auto se = elements.begin();
		while (se != elements.end()) {//cleanup loop
			if (!se->active) se = elements.erase(se);
			else se++;
		}

		mixer->update(dTime);
	}
	void Sequencer::writeTo(Mixer * m) {
		mixer = m;
	}
	Sequencer::SequencerElement::SequencerElement(Synthesizer::Instrument* i, double beatTiming, float freq) {
			time = beatTiming;
			note = i->play(freq, 10.0);
			active = true;
		}

	AudioSystem::AudioSystem() {
		SDL_Init(SDL_INIT_AUDIO);

		SDL_AudioSpec pref, real;
		outputMixer = new Mixer();
		testSequencer = new Sequencer();
		testSequencer->writeTo(outputMixer);

		SDL_zero(pref);
		pref.freq = hardwareInfo.soundCardInfo.outputHertz;
		pref.format = hardwareInfo.soundCardInfo.outputBitDepth;
		pref.channels = hardwareInfo.soundCardInfo.outputNumChannels;
		pref.samples = hardwareInfo.soundCardInfo.outputSamplesPerChannel;
		pref.callback = callbackForSDL;
		pref.userdata = outputMixer;

		playback = SDL_OpenAudioDevice(NULL, 0, &pref, &real, 0);
		//still a todo: figure out how to compensate for format changes
		//idea: do all calculations in float (double?) space and convert when finalizing mix
		//format only then matters when handing the values off to the driver, so we have to convert for the callback.
		//Convert between callbacks, there's enough time.

	}
	AudioSystem::~AudioSystem() {
		delete testSequencer;
		delete outputMixer;
		stopPlayback();
		SDL_CloseAudioDevice(playback);
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
}