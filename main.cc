/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "ns/noise_suppressor.h"

#include <memory>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "timing.h"

//采用https://github.com/mackron/dr_libs/blob/master/dr_wav.h 解码
#define DR_WAV_IMPLEMENTATION

#include "dr_wav.h"

#ifndef nullptr
#define nullptr 0
#endif

#ifndef MIN
#define MIN(A, B)        ((A) < (B) ? (A) : (B))
#endif

#ifndef MAX
#define MAX(A, B)        ((A) > (B) ? (A) : (B))
#endif

//写wav文件
void wavWrite_s16(char *filename, int16_t *buffer, size_t sampleRate, size_t totalSampleCount, unsigned int channels) {
    drwav_data_format format;
    format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
    format.channels = channels;
    format.sampleRate = (drwav_uint32) sampleRate;
    format.bitsPerSample = sizeof(*buffer) * 8;
    format.format = DR_WAVE_FORMAT_PCM;
    drwav wav;
    drwav_init_file_write(&wav, filename, &format, NULL);
    drwav_uint64 samplesWritten = drwav_write_pcm_frames(&wav, totalSampleCount, buffer);
    drwav_uninit(&wav);
    if (samplesWritten != totalSampleCount) {
        fprintf(stderr, "ERROR\n");
        exit(1);

    }
}

//读取wav文件
short *wavRead_s16(char *filename, uint32_t *sampleRate, uint64_t *totalSampleCount, unsigned int *channels) {
    short *buffer = drwav_open_file_and_read_pcm_frames_s16(filename, channels, sampleRate, totalSampleCount, NULL);
    if (buffer == NULL) {
        printf("ERROR.");
    }
    return buffer;
}

//分割路径函数
void splitpath(const char *path, char *drv, char *dir, char *name, char *ext) {
    const char *end;
    const char *p;
    const char *s;
    if (path[0] && path[1] == ':') {
        if (drv) {
            *drv++ = *path++;
            *drv++ = *path++;
            *drv = '\0';
        }
    } else if (drv)
        *drv = '\0';
    for (end = path; *end && *end != ':';)
        end++;
    for (p = end; p > path && *--p != '\\' && *p != '/';)
        if (*p == '.') {
            end = p;
            break;
        }
    if (ext)
        for (s = end; (*ext = *s++);)
            ext++;
    for (p = end; p > path;)
        if (*--p == '\\' || *p == '/') {
            p++;
            break;
        }
    if (name) {
        for (s = p; s < end;)
            *name++ = *s++;
        *name = '\0';
    }
    if (dir) {
        for (s = path; s < p;)
            *dir++ = *s++;
        *dir = '\0';
    }
}

using namespace webrtc;

int nsProc(short *input, size_t SampleCount, size_t sampleRate, int num_channels) {
    AudioBuffer audio(sampleRate, num_channels, sampleRate, num_channels, sampleRate,
                      num_channels);
    StreamConfig stream_config(sampleRate, num_channels);
    NsConfig cfg;
    /*
     * NsConfig::SuppressionLevel::k6dB
     * NsConfig::SuppressionLevel::k12dB
     * NsConfig::SuppressionLevel::k18dB
     * NsConfig::SuppressionLevel::k21dB
     */
//    cfg.target_level = NsConfig::SuppressionLevel::k21dB;
    NoiseSuppressor ns(cfg, sampleRate, num_channels);
    short *buffer = input;
    bool split_bands = sampleRate > 16000;
    uint64_t frames = (SampleCount / stream_config.num_frames());
    for (size_t frame_index = 0; frame_index < frames; ++frame_index) {
        audio.CopyFrom(buffer, stream_config);
        if (split_bands) {
            audio.SplitIntoFrequencyBands();
        }
        ns.Analyze(audio);
        ns.Process(&audio);
        if (split_bands) {
            audio.MergeFrequencyBands();
        }
        audio.CopyTo(stream_config, buffer);
        buffer += stream_config.num_frames();
    }
    return 0;
}

void WebRtc_DeNoise(char *in_file, char *out_file) {
    uint32_t sampleRate = 0;
    uint64_t nSampleCount = 0;
    uint32_t channels = 1;
    short *data_in = wavRead_s16(in_file, &sampleRate, &nSampleCount, &channels);
    if (data_in != NULL) {
        double startTime = now();
        short *data_out = (short *) calloc(nSampleCount, sizeof(short));
        if (data_out != NULL) {
            nsProc(data_in, nSampleCount, sampleRate, channels);
            double time_interval = calcElapsed(startTime, now());
            printf("time interval: %d ms\n ", (int) (time_interval * 1000));
            wavWrite_s16(out_file, data_in, sampleRate, (uint32_t) nSampleCount, channels);
            free(data_out);
        }
        free(data_in);
    }
}


int main(int argc, char *argv[]) {
    printf("webrtc noise suppressor\n");
    printf("blog:http://cpuimage.cnblogs.com/\n");
    printf("email:gaozhihan@vip.qq.com\n");
    if (argc < 2) {
        printf("usage:\n");
        printf("./webrtc_ns input.wav\n");
        printf("or\n");
        printf("./webrtc_ns input.wav output.wav\n");
        return -1;
    }
    char *in_file = argv[1];

    if (argc > 2) {
        char *out_file = argv[2];
        WebRtc_DeNoise(in_file, out_file);
    } else {
        char drive[3];
        char dir[256];
        char fname[256];
        char ext[256];
        char out_file[1024];
        splitpath(in_file, drive, dir, fname, ext);
        sprintf(out_file, "%s%s%s_out%s", drive, dir, fname, ext);
        WebRtc_DeNoise(in_file, out_file);
    }
    printf("press any key to exit.\n");
    getchar();
    return 0;
}

