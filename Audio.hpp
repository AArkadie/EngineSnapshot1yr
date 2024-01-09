#pragma once

#include <SDL_audio.h>

#include "Assets.hpp"
#include "HardwareAcceleration.hpp"
//never forget: at the end of it all you're just filling a buffer with diaphragm position data
namespace SAGE {

	//const double TAU = M_PI * 2; //Goes in Math or Assets I feel

	enum VirtualChannelSpeakerPosition {
		MID_FRONT_LEFT,
		MID_FRONT_RIGHT,
		MID_FRONT_CENTER,
		SUBWOOFER,
		MID_BACK_LEFT,
		MID_BACK_RIGHT,
		MID_FRONT_CENTER_LEFT,
		MID_FRONT_CENTER_RIGHT,
		MID_BACK_CENTER,
		TRUE_LEFT,
		TRUE_RIGHT,
		OVERHEAD,
		UP_FRONT_LEFT,
		UP_FRONT_CENTER,
		UP_FRONT_RIGHT,
		UP_BACK_LEFT,
		UP_BACK_CENTER,
		UP_BACK_RIGHT,
		ALL_SPEAKER_POSITIONS
	};

	static std::array<glm::vec3, ALL_SPEAKER_POSITIONS> fixedSpeakerPositionNormals{
		glm::normalize(glm::vec3(-1.f,0.f,-1.f)),
		glm::normalize(glm::vec3(1.f, 0.f, -1.f)),
		glm::vec3(0.f,0.f,-1.f),//forward
		glm::vec3(0),//bass could be inside or pure down, depends on what kinds of crazy speaker configs are out there
		glm::normalize(glm::vec3(-1.f, 0.f, 1.f)),
		glm::normalize(glm::vec3(1.f, 0.f, 1.f)),
		glm::normalize(glm::vec3(-1.f, 0.f, -2.f)),
		glm::normalize(glm::vec3(1.f, 0.f, -2.f)),
		glm::vec3(0.f, 0.f, 1.f),//back
		glm::vec3(-1.f, 0.f, 0.f),//left
		glm::vec3(1.f, 0.f, 0.f),//right
		glm::vec3(0.f, 1.f, 0.f),//up
		glm::normalize(glm::vec3(-1.f,1.f,-1.f)),
		glm::normalize(glm::vec3(0.f,1.f,-1.f)),
		glm::normalize(glm::vec3(1.f,1.f,-1.f)),
		glm::normalize(glm::vec3(-1.f, 1.f, 1.f)),
		glm::normalize(glm::vec3(0.f, 1.f, 1.f)),
		glm::normalize(glm::vec3(1.f, 1.f, 1.f))
	};//Think I'd want to do defines for these?

	static float vspd(VirtualChannelSpeakerPosition a, VirtualChannelSpeakerPosition b) {
		return glm::dot(fixedSpeakerPositionNormals[a], fixedSpeakerPositionNormals[b]);
	}

	static VirtualChannelSpeakerPosition getFixedPosition(glm::vec3 vectorTo) {
		size_t returner = SUBWOOFER;
		float maxDot = -1.f;
		for (size_t i = 0; i < ALL_SPEAKER_POSITIONS; i++) {
			float tempDot = glm::dot(glm::normalize(vectorTo), fixedSpeakerPositionNormals[i]);
			if (tempDot > maxDot) {
				returner = i;
				maxDot = tempDot;
			}
		}
		return (VirtualChannelSpeakerPosition)returner;
	}

	/*
	Instruments will become assets, but first we need to work out creating them by
	defining their constituent frequencies a-la fourier series and then we can put those
	series into a file and compile the wave table into a binary from that.  All sampled
	at 96k or even higher and then downsampled on load depending on the user's audio device.

	Other other idea: Instruments are game objects that invoke the synthesizer to make sound
	by marrying it with a control scheme.  Our goal, then, should not be to think of notes
	and instruments here, but rather the technical side of how we represent data streams
	that get transformed into sound.
	*/

	struct VirtualChannelUnit {
		std::array<float, ALL_SPEAKER_POSITIONS> values;
		VirtualChannelUnit operator+(const VirtualChannelUnit&);
	};

	struct MixerDirectInput {//will become a struct to just hold buffer data in virtual channel space
		//for now though, let's load it up for main testing
		std::vector<std::vector<float>> channelBuffers;
		double playbackCursor;
		size_t loopBeg, loopEnd;//note: denotes indices so loopEnd should go to size - 1;
		float playbackFrequency;//if this goes negative, we can reverse :)
		float playbackSpeed;
		float playbackVolume;
		bool isPaused, isMuted;

		float runPlayback(size_t);

		void togglePlay() { isPaused = !isPaused; }//play/pause
		void stop() {
			isPaused = true;
			playbackCursor = 0;
		}
		void setVolumeFactor(double v) { playbackVolume = fmin(fmax(0.0, v), 1.0); }//make sure to guard against negative values and high values that may cause damage
		void toggleMute() { isMuted = !isMuted; }
		void reverse() { playbackFrequency = -playbackFrequency; }//make frequency negative
		void skipSamples(int step) { playbackCursor += step; }//both forward and back.  Do we want to be able to skip by time?
		//consider skipTime() here
		void dilate(double playbackSpeedChangeFactor) { playbackSpeed *= playbackSpeedChangeFactor; }
		//will have to make time versions of these...probably better that way
		void moveLoopStart(size_t x) {
			if (x < loopEnd) loopBeg = std::max((size_t)0, x);
		}
		void moveLoopEnd(size_t x) {
			if (x > loopBeg) loopEnd = std::min(x, channelBuffers[0].size() - 1);
		}
		void setLoopRange(size_t start, size_t end) {
			moveLoopStart(start);
			moveLoopEnd(end);
		}
	};

	struct AudioSource {
		MixerDirectInput* data;
		double playbackCursor;
		size_t loopBeg, loopEnd;
		float playbackStride;
		float playbackSpeed;
		float playbackVolume;
		bool isPaused, isMuted;

		void SetSource(MixerDirectInput*);
	};

	struct SoundController {
		VirtualChannelUnit getSample(AudioSource*);
		void togglePlay(AudioSource*);
		void stop(AudioSource*);
		void setVolumeFactor(AudioSource*, double);
		void toggleMute(AudioSource*);
		void reverse(AudioSource*);
		void skipSamples(AudioSource*, int);//do we even need?
		void skipTime(AudioSource*, int ms);
		void dilate(AudioSource*, double);
		void moveLoopStart(AudioSource*, size_t);
		void moveLoopEnd(AudioSource*, size_t);
		void setLoopRange(AudioSource*, size_t, size_t);
	};

	/* Begin old audio stuff (mixer's being updated)*/

	//need a vector of instuments somewhere, call it a band.
	struct Synthesizer {
	
		Synthesizer() = default;
		~Synthesizer() = default;

		//note: ft = frequency * time
		//Sin
		static float oscSin(float freq, float time) {
			return sin(M_PI * 2 * freq * time);
		}
		//Triangle
		static float oscTriangle(float freq, float time) {
			return asin(oscSin(freq, time)) * 2.f / M_PI;
			//return 4 * abs((freq * time) - floor((freq * time) + 1.f / 2)) - 1;
		}
		//Square
		static float oscSquare(float freq, float time) {//= (2pi * ft < pi) ? + : - (requires wrapping)
			return oscSin(freq, time) > 0 ? 1.f : -1.f;
		}
		//Sawtooth
		static float oscSawtooth(float freq, float time) {
			return (2.f / M_PI) * (freq * M_PI * fmod(time, 1.0 / freq)) - M_PI / 2.f;
		}
		//Pulse wave will have to be implemented if needed but for now just know that it exists.
		//also white noise is useful but we need randomness for that

		//refactor Envelopes to be a class hierarchy and work on state machines.
		enum EnvelopeState {
			SILENT,
			ATTACK,
			DECAY,
			SUSTAIN,
			RELEASE
		};

		struct ADSREnvelope {
			float attackTime, decayTime, sustainTime, releaseTime;
			float buildup, fall, stay, dropoff;
			float maxVol, susVol;
			double lifetime;
			bool playing, drone;

			ADSREnvelope();//rest
			ADSREnvelope(float a, float d);//percussion/pluck string
			ADSREnvelope(float a, float d, float s, float r, bool cont);//wind/drawn string

			double getAmplitude(double dTime);
		};

		struct Note {
			double* waveTable;
			ADSREnvelope envelope;
			float frequency;
			unsigned int lifetime, tableSize;

			Note();
			~Note() = default;

			double sound(double dTime);
		};

		struct Instrument {
		
			Instrument();
			virtual ~Instrument() = default;

			Note play() { return Note(); }
			virtual Note play(short note, double duration) = 0;

			virtual void generateTimbre() = 0;
			void tune(float freq) { baseFrequency = freq; }

		protected:
			std::vector<double> timbre;//should be a vector of oscillators?  Which then produce a table...jury's out
			float baseFrequency, attackDuration, decayDuration, releaseDuration;
		};

		struct PlaybackInstrument : public Instrument {
			PlaybackInstrument(External_WaveFileData&);

			Note play(short n, double d) override;
		private:
			void generateTimbre() override;
		};

		struct SinInstrument : public Instrument {
		
			SinInstrument();
			//~SinInstrument() override{};

			Note play(short n, double d) override;
		private:
			void generateTimbre() override;
		};

		class silence : public Instrument {
			void generateTimbre() {
				for (double d : timbre) {
					d = 0.0;
				}
			}
		public:
			Note play(short n, double d) override {
				return Instrument::play();
			}
		};
	};

	//these are untested but...I assume they all work :)
	static void convertOutputUint8(  std::vector<float>&, Uint8*);
	static void convertOutputSint8(  std::vector<float>&, Uint8*);
	static void convertOutputUint16( std::vector<float>&, Uint8*);
	static void convertOutputSint16( std::vector<float>&, Uint8*);
	static void convertOutputUint32( std::vector<float>&, Uint8*);
	static void convertOutputSint32( std::vector<float>&, Uint8*);
	static void convertOutputUint64( std::vector<float>&, Uint8*);
	static void convertOutputSint64( std::vector<float>&, Uint8*);
	static void convertOutputFloat32(std::vector<float>&, Uint8*);

	//mixer takes all audio data for a frame and decides how to write it to the buffer
	//needs an internal function that has a mixing queue where it sums all sorts of information before writing to a channel
	struct Mixer {
	
		Mixer();
		~Mixer() = default;

		void recordFromSynth(Synthesizer::Note&);
		void recordFromFile(MixerDirectInput*);

		void update(double);
		bool channelsFree();

		void prepareOutput();

		void outputSound(Uint8* outputStream);

		//mixer needs some function integrate() which maps the virtual channel space onto actual channels
		//we'll sum all virtual channels before calling so we only have to integrate once (per frame).
		void integrate(VirtualChannelUnit&);
	private:
		struct AudioChannel {
		
			AudioChannel(int bufferSize);
			AudioChannel() = default;
			~AudioChannel() = default;

			float read();
			void write(float value);
			bool canWrite(int values);
			bool canRead() { return readIdle; }
			void writeLock(bool b) { writeIdle = b; }
			void readLock(bool b) {	readIdle = b; }
		private:
			std::vector<float> buffer;
			int readHead, writeHead;
			bool readIdle, writeIdle;//will need some way to encode positional information
		};
		float volume;
		double audioDeltaTime;
		std::vector<AudioChannel> channels;//rename outputChannels if we end up going with channels of input
		std::vector<float> outBuffer;
		std::vector<Synthesizer::Note> activeNotes;//both of these become a vector of pointers to structs
		std::vector<MixerDirectInput*> precompSounds;//with a function that can return samples
		//but not necessarily the data itself
		void(*conversionFunction)(std::vector<float>&, Uint8*);
	};

	//sequencer times when to send notes to the mixer and can also time engine events
	//or are notes just another type of event?
	struct Sequencer {
	
		struct SequencerElement {
			double time;
			Synthesizer::Note note;
			bool active;

			SequencerElement(Synthesizer::Instrument* i, double beatTiming, float freq);
		};
		Sequencer();
		~Sequencer() = default;

		void add(Synthesizer::Instrument* i, int beatNum, float freq);

		void update(double dTime);

		void writeTo(Mixer* m);
	private:
		Mixer* mixer;
		std::vector<SequencerElement> elements;
		unsigned short bpm;
		double beatTime, elapsedTime, elapsedBeats;
	};

	static void callbackForSDL(void* userData, Uint8* stream, int length) {
		Mixer* chex = (Mixer*)userData;
		chex->outputSound(stream);
	}

	struct AudioSystem {
		AudioSystem();
		~AudioSystem();

		SDL_AudioDeviceID playback;

		Mixer* outputMixer;
		Sequencer* testSequencer;

		void beginPlayback() { SDL_PauseAudioDevice(playback, 0); }
		void stopPlayback() { SDL_PauseAudioDevice(playback, 1); }
	};
}