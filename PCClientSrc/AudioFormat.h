#pragma once
#include <cstdint>
#include <fstream>

typedef struct  WAV_HEADER
{
    /* RIFF Chunk Descriptor */
    uint8_t         RIFF[4];        // RIFF Header Magic header
    uint32_t        ChunkSize;      // RIFF Chunk Size
    uint8_t         WAVE[4];        // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t         fmt[4];         // FMT header
    uint32_t        Subchunk1Size;  // Size of the fmt chunk
    uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
    uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
    uint32_t        bytesPerSec;    // bytes per second
    uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    uint16_t        bitsPerSample;  // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t         Subchunk2ID[4]; // "data"  string
    uint32_t        Subchunk2Size;  // Sampled data length
} wav_hdr;

class AudioFormat 
{
public:
	AudioFormat();

	// Setting format of sample frequency, bit resolution and number of channels 
	int SetFormat(uint32_t frequency, uint16_t resolution, uint16_t channels);
	
	// For testing
	int Create16BitStereoSineWave(char *audio, int size);
	int Create32BitStereoSineWave(char *audio, int size);
	void Gain32Bit(int *audio, int size, int gain);


	// Save Wave file by fileName with pointer to audio buffer of byte size
	int SaveWaveFile(char *fileName, char *audio, int size);

	// Save Wave file in chuncks
	int SaveWaveHeader(char *fileName);
	int SaveWaveBody(char *audio, int size);
	int SaveWaveTrailer(void);

private:
	wav_hdr WaveHeader;
	std::ofstream* fstream;
	size_t data_chunk_pos;
};
