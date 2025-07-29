#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_SAMPLES 100000
#define MA_WINDOW 10             // Wider window for better smoothing
#define THRESHOLD 1.0            // Threshold for wavelet noise removal
#define SKIP 10                  // Skip initial samples for bandwidth

// Calibration constants
#define ADC_MAX 2147483648.0
#define ZERO_CAL 0.0182303525507
#define SCALE_CAL 0.00000452466566

double adc_data[MAX_SAMPLES];
double weights[MAX_SAMPLES];
double ma_output[MAX_SAMPLES];
double approx[MAX_SAMPLES / 2];
double detail[MAX_SAMPLES / 2];
double reconstructed[MAX_SAMPLES];
double wavelet_weights[MAX_SAMPLES];
int sample_count = 0;

// ------------------------------------------------------------
// Read ADC values and convert to weights
// ------------------------------------------------------------
int read_adc_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }
    int i = 0;
    int adc_val;
    while (fscanf(fp, "%d", &adc_val) == 1 && i < MAX_SAMPLES) {
        adc_data[i] = (double)adc_val / ADC_MAX;
        weights[i] = (adc_data[i] - ZERO_CAL) / SCALE_CAL;
        i++;
    }
    fclose(fp);
    return (i % 2 == 0) ? i : i - 1; // Make even
}

// ------------------------------------------------------------
// Save signal to file
// ------------------------------------------------------------
void save_signal(const char *filename, const double *data, int len) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("File write failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < len; i++) {
        fprintf(fp, "%.10f\n", data[i]);
    }
    fclose(fp);
}

// ------------------------------------------------------------
// Centered Moving Average Filter
// ------------------------------------------------------------
void apply_moving_average() {
    int half = MA_WINDOW / 2;
    for (int i = 0; i < sample_count; i++) {
        double sum = 0.0;
        int count = 0;
        for (int j = -half; j <= half; j++) {
            int idx = i + j;
            if (idx >= 0 && idx < sample_count) {
                sum += weights[idx];
                count++;
            }
        }
        ma_output[i] = sum / count;
    }
    save_signal("ma_weights.txt", ma_output, sample_count);
}

// ------------------------------------------------------------
// Haar DWT
// ------------------------------------------------------------
void haar_dwt(const double *signal, int n, double *cA, double *cD) {
    for (int i = 0; i < n / 2; i++) {
        cA[i] = (signal[2 * i] + signal[2 * i + 1]) / sqrt(2.0);
        cD[i] = (signal[2 * i] - signal[2 * i + 1]) / sqrt(2.0);
    }
}

// ------------------------------------------------------------
// Thresholding of detail coefficients
// ------------------------------------------------------------
void threshold_detail(double *cD, int n, double threshold) {
    for (int i = 0; i < n; i++) {
        if (fabs(cD[i]) < threshold) {
            cD[i] = 0.0;
        }
    }
}

// ------------------------------------------------------------
// Inverse Haar DWT
// ------------------------------------------------------------
void haar_idwt(const double *cA, const double *cD, int n, double *recon) {
    for (int i = 0; i < n; i++) {
        recon[2 * i]     = (cA[i] + cD[i]) / sqrt(2.0);
        recon[2 * i + 1] = (cA[i] - cD[i]) / sqrt(2.0);
    }
}

// ------------------------------------------------------------
// Bandwidth Calculation (Max - Min after skip)
// ------------------------------------------------------------
double calc_bandwidth(const double *data, int len, int skip, double *min_val, double *max_val) {
    if (len <= skip) return 0.0;

    double min = data[skip];
    double max = data[skip];
    for (int i = skip; i < len; i++) {
        if (data[i] < min) min = data[i];
        if (data[i] > max) max = data[i];
    }
    *min_val = min;
    *max_val = max;
    return max - min;
}

// ------------------------------------------------------------
// Main Function
// ------------------------------------------------------------
int main() {
    sample_count = read_adc_file("adc_values60.txt");  // exarct your file name
    printf("Read %d samples from ADC file.\n", sample_count);

    // Step 1: Moving Average
    apply_moving_average();

    // Step 2: Apply Wavelet on MA output
    haar_dwt(ma_output, sample_count, approx, detail);

    threshold_detail(detail, sample_count / 2, THRESHOLD);

    haar_idwt(approx, detail, sample_count / 2, reconstructed);

    for (int i = 0; i < sample_count; i++) {
        wavelet_weights[i] = reconstructed[i];
    }

    save_signal("wavelet_weights.txt", wavelet_weights, sample_count);

    // Step 3: Bandwidth Analysis
    double min_orig, max_orig, min_ma, max_ma, min_wave, max_wave;
    double bw_orig = calc_bandwidth(weights, sample_count, SKIP, &min_orig, &max_orig);
    double bw_ma   = calc_bandwidth(ma_output, sample_count, SKIP, &min_ma, &max_ma);
    double bw_wave = calc_bandwidth(wavelet_weights, sample_count, SKIP, &min_wave, &max_wave);

    // Final Output
    printf("\n--- Bandwidth Results (Weights) ---\n");
    printf("Original  BW = %.6f (Min=%.6f, Max=%.6f)\n", bw_orig, min_orig, max_orig);
    printf("MA Filter BW = %.6f (Min=%.6f, Max=%.6f)\n", bw_ma, min_ma, max_ma);
    printf("Wavelet   BW = %.6f (Min=%.6f, Max=%.6f)\n", bw_wave, min_wave, max_wave);

    printf("\nSaved files: ma_weights.txt, wavelet_weights.txt\n");
    return 0;
}
