/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// If your target is limited in memory remove this macro to save 10K RAM
#define EIDSP_QUANTIZE_FILTERBANK   0

/**
 * Define the number of slices per model window.
 */
#define EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW 2 // 减少到 2，片段时长 1 秒

/* Includes ---------------------------------------------------------------- */
#include <PDM.h>
#include <HomeDeviceDetector_inferencing.h>

// LED pin definitions
#define LED_NORMAL D2      // 绿灯 - 正常状态
#define LED_SMOKEALARM D3  // 红灯 - 烟雾报警器
#define LED_DOORBELL D4    // 黄灯 - 门铃

// 设置分类判断的阈值
#define CONFIDENCE_THRESHOLD 0.8  // 60%的置信度阈值

/** Audio buffers, pointers and selectors */
typedef struct {
    signed short *buffers[2];
    unsigned char buf_select;
    unsigned char buf_ready;
    unsigned int buf_count;
    unsigned int n_samples;
} inference_t;

static inference_t inference;
static bool record_ready = false;
static signed short *sampleBuffer;
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);

// 平滑预测结果
static float normal_prob_avg = 0.0;
static float doorbell_prob_avg = 0.0;
static float smokealarm_prob_avg = 0.0;
static int prediction_count = 0;

/**
 * @brief      Arduino setup function
 */
void setup()
{
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo (Continuous Monitoring with LED)");

    // 初始化LED引脚
    pinMode(LED_NORMAL, OUTPUT);
    pinMode(LED_SMOKEALARM, OUTPUT);
    pinMode(LED_DOORBELL, OUTPUT);
    
    // 初始状态下关闭所有LED
    digitalWrite(LED_NORMAL, LOW);
    digitalWrite(LED_SMOKEALARM, LOW);
    digitalWrite(LED_DOORBELL, LOW);

    // 打印推理设置
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: %.2f ms.\n", (float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) /
                                            sizeof(ei_classifier_inferencing_categories[0]));

    run_classifier_init();
    if (microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE) == false) {
        ei_printf("ERR: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
        return;
    }
}

/**
 * @brief      更新LED状态
 */
void update_leds(float normal_prob, float doorbell_prob, float smokealarm_prob) {
    if (normal_prob >= CONFIDENCE_THRESHOLD) {
        digitalWrite(LED_NORMAL, HIGH);
    } else {
        digitalWrite(LED_NORMAL, LOW);
    }
    
    if (doorbell_prob >= CONFIDENCE_THRESHOLD) {
        digitalWrite(LED_DOORBELL, HIGH);
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_DOORBELL, LOW);
            delay(100);
            digitalWrite(LED_DOORBELL, HIGH);
            delay(100);
        }
    } else {
        digitalWrite(LED_DOORBELL, LOW);
    }
    
    if (smokealarm_prob >= CONFIDENCE_THRESHOLD) {
        digitalWrite(LED_SMOKEALARM, HIGH);
        for (int i = 0; i < 5; i++) {
            digitalWrite(LED_SMOKEALARM, LOW);
            delay(50);
            digitalWrite(LED_SMOKEALARM, HIGH);
            delay(50);
        }
    } else {
        digitalWrite(LED_SMOKEALARM, LOW);
    }
}

/**
 * @brief      Arduino main function. Runs the inferencing loop.
 */
void loop()
{
    bool m = microphone_inference_record();
    if (!m) {
        ei_printf("ERR: Failed to record audio...\n");
        return;
    }

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
    signal.get_data = microphone_audio_signal_get_data; // 修正为正确的函数名
    ei_impulse_result_t result = {0};

    EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", r);
        return;
    }

    // 获取当前片段的预测概率
    float normal_prob = 0.0;
    float doorbell_prob = 0.0;
    float smokealarm_prob = 0.0;

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (strcmp(result.classification[ix].label, "normal") == 0) {
            normal_prob = result.classification[ix].value;
        }
        else if (strcmp(result.classification[ix].label, "doorbell") == 0) {
            doorbell_prob = result.classification[ix].value;
        }
        else if (strcmp(result.classification[ix].label, "smokeAlarm") == 0) {
            smokealarm_prob = result.classification[ix].value;
        }
    }

    // 平滑预测结果
    normal_prob_avg = (normal_prob_avg * prediction_count + normal_prob) / (prediction_count + 1);
    doorbell_prob_avg = (doorbell_prob_avg * prediction_count + doorbell_prob) / (prediction_count + 1);
    smokealarm_prob_avg = (smokealarm_prob_avg * prediction_count + smokealarm_prob) / (prediction_count + 1);
    prediction_count++;
    if (prediction_count >= EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW) {
        prediction_count = 0;
    }

    // 只有在完整窗口（2 个片段）后才更新 LED 和打印结果
    if (++print_results >= EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW) {
        // 打印预测结果（仅当检测到目标类别时）
        bool print = false;
        if (doorbell_prob_avg >= CONFIDENCE_THRESHOLD || smokealarm_prob_avg >= CONFIDENCE_THRESHOLD) {
            print = true;
        }
        if (print) {
            ei_printf("Predictions ");
            ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
                result.timing.dsp, result.timing.classification, result.timing.anomaly);
            ei_printf(": \n");
            ei_printf("    normal: %.5f\n", normal_prob_avg);
            ei_printf("    doorbell: %.5f\n", doorbell_prob_avg);
            ei_printf("    smokeAlarm: %.5f\n", smokealarm_prob_avg);
        }
        print_results = 0;

        // 更新 LED 状态
        update_leds(normal_prob_avg, doorbell_prob_avg, smokealarm_prob_avg);
    }
}

/**
 * @brief      PDM buffer full callback
 */
static void pdm_data_ready_inference_callback(void)
{
    int bytesAvailable = PDM.available();

    // read into the sample buffer
    int bytesRead = PDM.read((char *)&sampleBuffer[0], bytesAvailable);

    if (record_ready == true) {
        for (int i = 0; i < bytesRead >> 1; i++) {
            inference.buffers[inference.buf_select][inference.buf_count++] = sampleBuffer[i];

            if (inference.buf_count >= inference.n_samples) {
                inference.buf_select ^= 1;
                inference.buf_count = 0;
                inference.buf_ready = 1;
            }
        }
    }
}

/**
 * @brief      Init inferencing struct and setup/start PDM
 */
static bool microphone_inference_start(uint32_t n_samples)
{
    inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));
    if (inference.buffers[0] == NULL) {
        return false;
    }

    inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));
    if (inference.buffers[1] == NULL) {
        free(inference.buffers[0]);
        return false;
    }

    sampleBuffer = (signed short *)malloc((n_samples >> 1) * sizeof(signed short));
    if (sampleBuffer == NULL) {
        free(inference.buffers[0]);
        free(inference.buffers[1]);
        return false;
    }

    inference.buf_select = 0;
    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    // configure the data receive callback
    PDM.onReceive(&pdm_data_ready_inference_callback);

    PDM.setBufferSize(n_samples * sizeof(int16_t)); // 增大 PDM 缓冲区

    if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) {
        ei_printf("Failed to start PDM!");
    }

    PDM.setGain(127);

    record_ready = true;

    return true;
}

/**
 * @brief      Wait on new data
 */
static bool microphone_inference_record(void)
{
    bool ret = true;

    if (inference.buf_ready == 1) {
        ei_printf(
            "Error sample buffer overrun. Decrease the number of slices per model window "
            "(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)\n");
        ret = false;
    }

    while (inference.buf_ready == 0) {
        delay(1);
    }

    inference.buf_ready = 0;

    return ret;
}

/**
 * Get raw audio signal data
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);
    return 0;
}

/**
 * @brief      Stop PDM and release buffers
 */
static void microphone_inference_end(void)
{
    PDM.end();
    free(inference.buffers[0]);
    free(inference.buffers[1]);
    free(sampleBuffer);
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif