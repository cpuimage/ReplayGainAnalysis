
#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//采用https://github.com/mackron/dr_libs/blob/master/dr_wav.h 解码
#define DR_WAV_IMPLEMENTATION

#include "dr_wav.h"
#include "gain_analysis.h"


#ifndef min
#define min(a, b)            (((a) < (b)) ? (a) : (b))
#endif

//读取wav文件
int16_t *wavRead_int16(char *filename, uint32_t *sampleRate, uint64_t *totalSampleCount) {
    unsigned int channels;
    int16_t *buffer = drwav_open_and_read_file_s16(filename, &channels, sampleRate, totalSampleCount);
    if (buffer == NULL) {
        printf("读取wav文件失败.");
    }
    //仅仅处理单通道音频
    if (channels != 1) {
        drwav_free(buffer);
        buffer = NULL;
        *sampleRate = 0;
        *totalSampleCount = 0;
    }
    return buffer;
}


float getGaindB(int16_t *buffer, size_t totalSampleCount, int sampleRate, size_t analyzeSamples) {
    float ret = -0.00000000001f;
    if (totalSampleCount == 0) return ret;
    if (analyzeSamples == 0) return ret;
    const int maxSamples = 2400;
    analyzeSamples = min(maxSamples, analyzeSamples);
    ret = 1.0f;
    int num_channels = 1;
    Float_t inf_buffer[maxSamples];
    size_t totalCount = totalSampleCount / analyzeSamples;
    if (InitGainAnalysis(sampleRate) == INIT_GAIN_ANALYSIS_OK) {
        int16_t *input = buffer;
        for (int i = 0; i < totalCount; i++) {
            for (int n = 0; n < analyzeSamples; n++) {
                inf_buffer[n] = input[n];
            }
            if (AnalyzeSamples(inf_buffer, NULL, analyzeSamples, num_channels) != GAIN_ANALYSIS_OK)
                break;
            GetTitleGain();
            //	printf("Recommended dB change for analyzeSamples %d: %+6.2f dB\n", i, titleGain);
            input += analyzeSamples;
        }
        ret = GetAlbumGain();
    }
    if ((int) ret == GAIN_NOT_ENOUGH_SAMPLES) {
        ret = -0.00000000001f;
    }
    return ret;
}


void analyze(char *in_file, int ref_ms) {
    uint32_t sampleRate = 0;
    uint64_t totalSampleCount = 0;
    int16_t *wavBuffer = wavRead_int16(in_file, &sampleRate, &totalSampleCount);
    if (wavBuffer != NULL) {
        size_t analyzeSamples = ref_ms * (sampleRate / 1000);
        float gain = getGaindB(wavBuffer, totalSampleCount, sampleRate, analyzeSamples);

        printf("recommended dB change: %f \n", gain);
        free(wavBuffer);
    }
}

int main(int argc, char *argv[]) {
    printf("Replay Gain Analysis\n");
    printf("blog:http://cpuimage.cnblogs.com/\n");
    if (argc < 2)
        return -1;
    char *in_file = argv[1];
    //指定分析长度1秒
    int ref_ms = 1000;
    analyze(in_file, ref_ms);
    getchar();
    printf("press any key to exit. \n");
    return 0;
}

#ifdef __cplusplus
}
#endif
