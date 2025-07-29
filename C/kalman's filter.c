#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define MAX_SAMPLES 100000
#define FILE_NAME "adc_values60.txt"

#define ZERO_CAL 0.01823035255075
#define SCALE_CAL 0.00000451794631
#define ADC_MAX 2147483648.0
#define FS 50.0
#define MA_WINDOW 10

#define Q 0.001f  // Kalman Process noise
#define R 1.0f    // Kalman Measurement noise

// ------------------------------------------------------
// Compute Bandwidth (max - min)
// ------------------------------------------------------
float compute_bandwidth(float* data, int len) {
    float min = data[0], max = data[0];
    for (int i = 1; i < len; i++) {
        if (data[i] < min) min = data[i];
        if (data[i] > max) max = data[i];
    }
    return max - min;
}

// ------------------------------------------------------
// Kalman Filter
// ------------------------------------------------------
void kalman_filter(float* input, float* output, int len) {
    float x_est = input[0];
    float p = 1.0f;

    for (int i = 0; i < len; i++) {
        float x_pred = x_est;
        float p_pred = p + Q;

        float K = p_pred / (p_pred + R);
        x_est = x_pred + K * (input[i] - x_pred);
        p = (1 - K) * p_pred;

        output[i] = x_est;
    }
}

// ------------------------------------------------------
// Main Program
// ------------------------------------------------------
int main() {
    FILE* f = fopen(FILE_NAME, "r");
    if (!f) {
        printf("Error opening file %s\n", FILE_NAME);
        return 1;
    }

    int32_t adc_raw[MAX_SAMPLES];
    float normalized[MAX_SAMPLES];
    float mov_filtered[MAX_SAMPLES];
    float kalman_filtered[MAX_SAMPLES];

    int N = 0;
    while (fscanf(f, "%d", &adc_raw[N]) == 1 && N < MAX_SAMPLES)
        N++;
    fclose(f);

    // Convert ADC to weights
    for (int i = 0; i < N; i++) {
        float data_in = (float)adc_raw[i] / ADC_MAX;
        normalized[i] = (data_in - ZERO_CAL) / SCALE_CAL;
    }

    // Compute original bandwidth
    float bw_orig = compute_bandwidth(normalized, N);

    // Moving Average Filter
    for (int i = 0; i < N; i++) {
        float sum = 0.0f;
        int count = 0;
        for (int j = i - MA_WINDOW / 2; j <= i + MA_WINDOW / 2; j++) {
            if (j >= 0 && j < N) {
                sum += normalized[j];
                count++;
            }
        }
        mov_filtered[i] = sum / count;
    }
    float bw_mov = compute_bandwidth(mov_filtered, N);

    // Kalman Filter
    kalman_filter(mov_filtered, kalman_filtered, N);
    float bw_kalman = compute_bandwidth(kalman_filtered, N);

    // Write original data
    FILE* f_orig = fopen("original_data.csv", "w");
    if (!f_orig) {
        printf("Error creating original_data.csv\n");
        return 1;
    }
    fprintf(f_orig, "Original\n");
    for (int i = 0; i < N; i++) {
        fprintf(f_orig, "%.6f\n", normalized[i]);
    }
    fclose(f_orig);

    // Write moving average data
    FILE* f_mov = fopen("mov_filtered_data.csv", "w");
    if (!f_mov) {
        printf("Error creating mov_filtered_data.csv\n");
        return 1;
    }
    fprintf(f_mov, "MOV_Filtered\n");
    for (int i = 0; i < N; i++) {
        fprintf(f_mov, "%.6f\n", mov_filtered[i]);
    }
    fclose(f_mov);

    // Write Kalman filtered data
    FILE* f_kalman = fopen("kalman_filtered_data.csv", "w");
    if (!f_kalman) {
        printf("Error creating kalman_filtered_data.csv\n");
        return 1;
    }
    fprintf(f_kalman, "Kalman_Filtered\n");
    for (int i = 0; i < N; i++) {
        fprintf(f_kalman, "%.6f\n", kalman_filtered[i]);
    }
    fclose(f_kalman);

    // Print Bandwidth Results
    printf("\nBandwidth (Original)        : %.5f\n", bw_orig);
    printf("Bandwidth (After MOV)       : %.5f\n", bw_mov);
    printf("Bandwidth (After Kalman)    : %.5f\n", bw_kalman);
    printf("Output files generated:\n");
    printf(" - original_data.csv\n");
    printf(" - mov_filtered_data.csv\n");
    printf(" - kalman_filtered_data.csv\n");

    return 0;
}
