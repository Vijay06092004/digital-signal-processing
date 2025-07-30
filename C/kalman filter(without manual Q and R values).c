#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define MAX_SAMPLES 100000
#define FILE_NAME "adc_values60.txt"

#define ZERO_CAL 0.01823035255075
#define SCALE_CAL 0.00000451794631
#define ADC_MAX 2147483648.0
#define MA_WINDOW 10
#define SKIP 10

// Function to estimate variance
float estimate_variance(float* data, int len) {
    float sum = 0.0f, sum_sq = 0.0f;
    for (int i = 0; i < len; i++) {
        sum += data[i];
        sum_sq += data[i] * data[i];
    }
    float mean = sum / len;
    return (sum_sq / len) - (mean * mean);
}

// Function to estimate measurement noise (R)
float estimate_measurement_noise(float* original, float* smoothed, int len) {
    float sum_sq_diff = 0.0f;
    for (int i = 0; i < len; i++) {
        float diff = original[i] - smoothed[i];
        sum_sq_diff += diff * diff;
    }
    return sum_sq_diff / len;
}

// Moving Average Filter
void moving_average(float* input, float* output, int len, int window) {
    for (int i = 0; i < len; i++) {
        float sum = 0.0f;
        int count = 0;
        for (int j = i - window / 2; j <= i + window / 2; j++) {
            if (j >= 0 && j < len) {
                sum += input[j];
                count++;
            }
        }
        output[i] = sum / count;
    }
}

// Kalman Filter
void kalman_filter(float* input, float* output, int len, float Q, float R) {
    float x = input[0];  // initial estimate
    float p = 1.0f;      // initial error covariance
    float k;

    for (int i = 0; i < len; i++) {
        // Prediction update
        p = p + Q;

        // Measurement update
        k = p / (p + R);
        x = x + k * (input[i] - x);
        p = (1 - k) * p;

        output[i] = x;
    }
}

// Bandwidth Calculation
float calculate_bandwidth(float* data, int len, int skip) {
    float min = data[skip], max = data[skip];
    for (int i = skip; i < len; i++) {
        if (data[i] < min) min = data[i];
        if (data[i] > max) max = data[i];
    }
    return max - min;
}

int main() {
    FILE* file = fopen(FILE_NAME, "r");
    if (!file) {
        printf("Error opening file\n");
        return 1;
    }

    int32_t adc[MAX_SAMPLES];
    float weights[MAX_SAMPLES];
    float mov_filtered[MAX_SAMPLES];
    float kalman_filtered[MAX_SAMPLES];
    int N = 0;

    // Read ADC data
    while (fscanf(file, "%d", &adc[N]) == 1 && N < MAX_SAMPLES) {
        N++;
    }
    fclose(file);

    // Convert ADC to weights
    for (int i = 0; i < N; i++) {
        weights[i] = (adc[i] / ADC_MAX - ZERO_CAL) / SCALE_CAL;
    }

    // Apply Moving Average
    moving_average(weights, mov_filtered, N, MA_WINDOW);

    // Estimate Q and R from data
    float R = estimate_measurement_noise(weights, mov_filtered, N);
    float Q = estimate_variance(mov_filtered, N);

    // Apply Kalman Filter
    kalman_filter(mov_filtered, kalman_filtered, N, Q, R);

    // Calculate Bandwidths
    float bw_original = calculate_bandwidth(weights, N, SKIP);
    float bw_ma = calculate_bandwidth(mov_filtered, N, SKIP);
    float bw_kalman = calculate_bandwidth(kalman_filtered, N, SKIP);

    // Output Results
    printf("Estimated Q: %.6f\n", Q);
    printf("Estimated R: %.6f\n", R);
    printf("Original Bandwidth      : %.4f\n", bw_original);
    printf("Moving Average Bandwidth: %.4f\n", bw_ma);
    printf("Kalman Filter Bandwidth : %.4f\n", bw_kalman);

    return 0;
}
