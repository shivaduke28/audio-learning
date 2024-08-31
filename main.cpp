#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cstring>
#include <memory>

uint32_t read4(std::istream &stream)
{
    char s[4];
    stream.read(s, sizeof s);
    return static_cast<uint32_t>(s[0]) |
           static_cast<uint32_t>(s[1]) << 8 |
           static_cast<uint32_t>(s[2]) << 16 |
           static_cast<uint32_t>(s[3]) << 24;
}

uint16_t read2(std::istream &stream)
{
    char s[2];
    stream.read(s, sizeof s);
    return static_cast<uint16_t>(s[0]) |
           static_cast<uint16_t>(s[1]) << 8;
}

class RiffHeader
{
public:
    uint32_t chunkSize;
    char formType[4];

    RiffHeader(std::istream &stream)
    {
        chunkSize = read4(stream);
        stream.read(formType, 4);
    }
};

struct FmtChunk
{
    uint32_t chunkSize;
    uint16_t waveFormatType;
    uint16_t channel;
    uint32_t samplesPerSec;
    uint32_t bytesPerSec;
    uint16_t blockSize;
    uint16_t bitsPerSample;

    FmtChunk(std::istream &stream)
    {
        chunkSize = read4(stream);
        waveFormatType = read2(stream);
        channel = read2(stream);
        samplesPerSec = read4(stream);
        bytesPerSec = read4(stream);
        blockSize = read2(stream);
        bitsPerSample = read2(stream);
    }
};

struct DataChunk
{
    uint32_t chunkSize;
    std::unique_ptr<char[]> data;

    DataChunk(std::istream &stream)
    {
        chunkSize = read4(stream);
        data.reset(new char[chunkSize]);
        stream.read(data.get(), chunkSize);
    }
};

struct UnknownChunk
{
    char chunkId[4];
    uint32_t chunkSize;

    UnknownChunk(std::istream &stream, std::array<char, 4> &chunkIdArr)
    {
        chunkSize = read4(stream);
        for (int i = 0; i < 4; i++)
        {
            chunkId[i] = chunkIdArr[i];
        }

        // FIXME: move position without reading.
        char data[chunkSize];
        stream.read(data, chunkSize);
    }
};

std::ostream &operator<<(std::ostream &os, RiffHeader const &header)
{
    char formType[5];
    std::strncpy(formType, header.formType, 4);

    os
        << "chunkId:   " << "RIFF" << std::endl
        << "chunkSize: " << header.chunkSize << std::endl
        << "formType:  " << formType << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, FmtChunk const &chunk)
{
    os
        << "chunkId:        " << "fmt " << std::endl
        << "chunkSize:      " << chunk.chunkSize << std::endl
        << "waveFormatType: " << chunk.waveFormatType << std::endl
        << "channel:        " << chunk.channel << std::endl
        << "samplesPerSec:  " << chunk.samplesPerSec << std::endl
        << "bytesPerSec:    " << chunk.bytesPerSec << std::endl
        << "blockSize:      " << chunk.blockSize << std::endl
        << "bitsPerSample:  " << chunk.bitsPerSample << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, DataChunk const &chunk)
{
    os
        << "chunkId:   " << "data" << std::endl
        << "chunkSize: " << chunk.chunkSize << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, UnknownChunk const &chunk)
{
    char chunkId[5];
    std::strncpy(chunkId, chunk.chunkId, 4);

    os
        << "chunkId:   " << chunkId << std::endl
        << "chunkSize: " << chunk.chunkSize << std::endl;
    return os;
}

const std::array<char, 4> ChunkIdRiff = {'R', 'I', 'F', 'F'};
const std::array<char, 4> ChunkIdFmt = {'f', 'm', 't', ' '};
const std::array<char, 4> ChunkIdData = {'d', 'a', 't', 'a'};

int main()
{
    std::ifstream file("audio.wav", std::ios::binary);
    if (!file)
    {
        std::cerr << "File not found." << std::endl;
        return 1;
    }

    while (file.good())
    {
        std::array<char, 4> chunkId;
        file.read(chunkId.data(), 4);

        if (chunkId == ChunkIdRiff)
        {
            RiffHeader riffHeader(file);
            std::cout << riffHeader << std::endl;
        }
        else if (chunkId == ChunkIdFmt)
        {
            FmtChunk fmtChunk(file);
            std::cout << fmtChunk << std::endl;
        }
        else if (chunkId == ChunkIdData)
        {
            DataChunk dataChunk(file);
            std::cout << dataChunk << std::endl;
            break;
        }
        else
        {
            UnknownChunk chunk(file, chunkId);
            std::cout << chunk << std::endl;
        }
    }

    std::cerr << "End" << std::endl;
    return 0;
}