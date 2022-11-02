// filesystem management functions
// void open_filesystem_and_ram(struct pi_device *flash, struct pi_device *fs);

// WAVE file header format
typedef struct {
    unsigned char riff[4];                      // RIFF string
    unsigned int FileSize;                      // overall size of file in bytes
    unsigned char wave[4];                      // WAVE string
    unsigned char fmt_chunk_marker[4];          // fmt string with trailing null char
    unsigned int lenght_of_fmt;                 // length of the format data
    unsigned int format_type;                   // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    unsigned int NumChannels;                   // no.of channels
    unsigned int SampleRate;                    // sampling rate (blocks per second)
    unsigned int byterate;                      // SampleRate * NumChannels * BitsPerSample/8
    unsigned int block_align;                   // NumChannels * BitsPerSample/8
    unsigned int BitsPerSample;                 // bits per sample, 8- 8bits, 16- 16 bits etc
    unsigned char data_chunk_header[4];         // DATA string or FLLR string
    unsigned int DataSize;                      // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} header_struct;

// int ReadWavFromFile(char *FileName, void* OutBuf, unsigned int BufSize, header_struct *HeaderInfo, struct pi_device fs);