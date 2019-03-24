
#include <cmath>
#include <string>
using namespace std;

#include "AudioFormat.h"

namespace little_endian_io
{
  template <typename Word>
  std::ostream& write_word( std::ostream& outs, Word value, unsigned size = sizeof( Word ) )
  {
    for (; size; --size, value >>= 8)
      outs.put( static_cast <char> (value & 0xFF) );
    return outs;
  }
}
using namespace little_endian_io;

AudioFormat::AudioFormat()
{
	SetFormat(44100, 16, 2); // Default format : rate = 44.1, bits = 16, stereo
}

int AudioFormat::SetFormat(uint32_t sampleFrequency, uint16_t sampleResolution, uint16_t channels)
{
	strncpy((char*)WaveHeader.RIFF, "RIFF", 4);
	WaveHeader.ChunkSize = 0; // Adjuste for every file save
	strncpy((char*)WaveHeader.WAVE, "WAVE", 4);
	strncpy((char*)WaveHeader.fmt,  "fmt ", 4);
	WaveHeader.Subchunk1Size = 16; // Standard size no extension
	WaveHeader.AudioFormat = 1; // PCM
	WaveHeader.NumOfChan = channels;
	WaveHeader.SamplesPerSec = sampleFrequency;
	WaveHeader.bytesPerSec = channels * (sampleFrequency * sampleResolution)/8;
	WaveHeader.blockAlign = channels * (sampleResolution/8);
	WaveHeader.bitsPerSample = sampleResolution;
	strncpy((char*)WaveHeader.Subchunk2ID, "data", 4);
	WaveHeader.Subchunk2Size = 0; // Adjuste for every file save
	
	return 0;
}

int AudioFormat::Create16BitStereoSineWave(char *audio, int size)
{
	short *sample = (short *)audio;
	const double two_pi = 6.283185307179586476925286766559;
	const double max_amplitude = 32760;  // "volume"

	double hz        = WaveHeader.SamplesPerSec; // samples per second
	double frequency = 261.626;  // middle C
	double seconds   = size/WaveHeader.bytesPerSec;      // time

	int N = (int)(hz * seconds);  // total number of samples
	int M = size/(WaveHeader.NumOfChan * (WaveHeader.bitsPerSample/8)); // total number of iterations

	for (int n = 0; n < N && M > 0; n++)
	{
		double amplitude = (double)n / N * max_amplitude;
		double value     = sin( (two_pi * n * frequency) / hz );
		*sample++ = (short)(amplitude  * value);
		*sample++ = (short)((max_amplitude - amplitude) * value);
		M--;
	}

	return 0;
}

int AudioFormat::Create32BitStereoSineWave(char *audio, int size)
{
	int *sample = (int *)audio;
	const double two_pi = 6.283185307179586476925286766559;
	const double max_amplitude = 2147483648;  // "volume"

	double hz        = WaveHeader.SamplesPerSec; // samples per second
	double frequency = 261.626;  // middle C
	double seconds   = size/WaveHeader.bytesPerSec;      // time

	int N = (int)(hz * seconds);  // total number of samples
	int M = size/(WaveHeader.NumOfChan * (WaveHeader.bitsPerSample/8)); // total number of iterations

	for (int n = 0; n < N && M > 0; n++)
	{
		double amplitude = (double)n / N * max_amplitude;
		double value     = sin( (two_pi * n * frequency) / hz );
		*sample++ = (int)(amplitude  * value);
		*sample++ = (int)((max_amplitude - amplitude) * value);
		M--;
	}

	return 0;
}

void AudioFormat::Gain32Bit(int *audio, int size, int gain)
{
	for (int i = 0; i < size; i++)
		audio[i] = audio[i]*gain;
}

int AudioFormat::SaveWaveHeader(char *fileName)
{
	fstream =  new ofstream(fileName, ios::binary);

	// Write the file headers
	*fstream << "RIFF----WAVEfmt ";  // (ChunkSize to be filled in later)
	write_word(*fstream, WaveHeader.Subchunk1Size, sizeof(WaveHeader.Subchunk1Size));  // no extension data
	write_word(*fstream, WaveHeader.AudioFormat, sizeof(WaveHeader.AudioFormat));  // PCM - integer samples
	write_word(*fstream, WaveHeader.NumOfChan, sizeof(WaveHeader.NumOfChan));  // two channels (stereo file)
	write_word(*fstream, WaveHeader.SamplesPerSec, sizeof(WaveHeader.SamplesPerSec));  // samples per second (Hz)
	write_word(*fstream, WaveHeader.bytesPerSec, sizeof(WaveHeader.bytesPerSec));  // (Sample Rate * BitsPerSample * Channels) / 8
	write_word(*fstream, WaveHeader.blockAlign, sizeof(WaveHeader.blockAlign));  // data block size (size of two integer samples, one for each channel, in bytes)
	write_word(*fstream, WaveHeader.bitsPerSample, sizeof(WaveHeader.bitsPerSample));  // number of bits per sample (use a multiple of 8)

																				// Write the data chunk header
	data_chunk_pos = (size_t)fstream->tellp();
	*fstream << "data----";  // (Subchunk2Size to be filled in later)
	
	return 0;
}

int AudioFormat::SaveWaveBody(char *audio, int size)
{
	// Write the audio samples
	for (int i = 0; i < size; i++) {
		fstream->put(audio[i]);
	}
	return size;
}

int AudioFormat::SaveWaveTrailer(void)
{
	// (We'll need the final file size to fix the chunk sizes above)
	size_t file_length = (size_t)fstream->tellp();

	// Fix the data chunk header to contain the data size
	WaveHeader.Subchunk2Size = file_length - data_chunk_pos + 8;
	fstream->seekp(data_chunk_pos + sizeof(WaveHeader.Subchunk2ID));
	write_word(*fstream, WaveHeader.Subchunk2Size, sizeof(WaveHeader.Subchunk2Size));

	// Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
	WaveHeader.ChunkSize = file_length - 8;
	fstream->seekp(sizeof(WaveHeader.RIFF));
	write_word(*fstream, WaveHeader.ChunkSize, sizeof(WaveHeader.ChunkSize));
 
	fstream->close();
	delete fstream;

	return 0;
}

int AudioFormat::SaveWaveFile(char *fileName, char *audio, int size)
{
	ofstream f( fileName, ios::binary );

	// Write the file headers
	f << "RIFF----WAVEfmt ";  // (ChunkSize to be filled in later)
	write_word( f,  WaveHeader.Subchunk1Size, sizeof(WaveHeader.Subchunk1Size) );  // no extension data
	write_word( f,  WaveHeader.AudioFormat, sizeof(WaveHeader.AudioFormat) );  // PCM - integer samples
	write_word( f,  WaveHeader.NumOfChan, sizeof(WaveHeader.NumOfChan) );  // two channels (stereo file)
	write_word( f,  WaveHeader.SamplesPerSec, sizeof(WaveHeader.SamplesPerSec) );  // samples per second (Hz)
	write_word( f,  WaveHeader.bytesPerSec, sizeof(WaveHeader.bytesPerSec) );  // (Sample Rate * BitsPerSample * Channels) / 8
	write_word( f,  WaveHeader.blockAlign, sizeof(WaveHeader.blockAlign) );  // data block size (size of two integer samples, one for each channel, in bytes)
	write_word( f,  WaveHeader.bitsPerSample, sizeof(WaveHeader.bitsPerSample) );  // number of bits per sample (use a multiple of 8)

	// Write the data chunk header
	size_t data_chunk_pos = (size_t)f.tellp();
	f << "data----";  // (Subchunk2Size to be filled in later)
  
	// Write the audio samples
	for (int i = 0; i < size; i++) {
		f.put(audio[i]); 
	}
  
	// (We'll need the final file size to fix the chunk sizes above)
	size_t file_length = (size_t)f.tellp();

	// Fix the data chunk header to contain the data size
	WaveHeader.Subchunk2Size = file_length - data_chunk_pos + 8;
	f.seekp( data_chunk_pos + sizeof(WaveHeader.Subchunk2ID) );
	write_word(f, WaveHeader.Subchunk2Size, sizeof(WaveHeader.Subchunk2Size));

	// Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
	WaveHeader.ChunkSize = file_length - 8;
	f.seekp( sizeof(WaveHeader.RIFF) );
	write_word(f, WaveHeader.ChunkSize, sizeof(WaveHeader.ChunkSize)); 

	return 0;
}