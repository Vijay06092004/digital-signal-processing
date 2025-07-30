 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include <math.h>
 #include <string.h>
 #define MAX_SAMPLES 10000
 #define FIR_LEN 21
 #define PI 3.141592653589793
 #define ADC_MAX 2147483648.0
 #define ZERO_CAL -0.0006981067708
 #define SCALE_CAL 0.00000452466566
 #define FIR_ORDER 10
 int32_t raw_adc[MAX_SAMPLES];
 double norm_data_in[MAX_SAMPLES];
 double weights[MAX_SAMPLES];
 int num_samples = 0;

 // Read and normalize ADC values, save normalized and weight
 void read_and_process_adc(const char* filename) {
    FILE *fp = fopen(filename, "r");
    FILE *norm_fp = fopen("normalized.txt", "w");
    FILE *weight_fp = fopen("weights.txt", "w");
    if (!fp || !norm_fp || !weight_fp) {
        perror("File error");
        exit(EXIT_FAILURE);
    }
    while (fscanf(fp, "%d", &raw_adc[num_samples]) == 1 && num_samples < MAX_SAMPLES) {
        norm_data_in[num_samples] = (double)raw_adc[num_samples] / ADC_MAX;
        weights[num_samples] = (norm_data_in[num_samples]-ZERO_CAL) / SCALE_CAL;
        fprintf(norm_fp, "%.10f\n", norm_data_in[num_samples]);
        fprintf(weight_fp, "%.10f\n", weights[num_samples]);
        num_samples++;
    }
    fclose(fp);
    fclose(norm_fp);
    fclose(weight_fp);
}
 // Moving average filter
 void moving_average(int window_size, const char *output_file) {
    FILE *fp = fopen(output_file, "w");
    if (!fp) {
        perror("Output file open failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_samples; i++) {
        double sum = 0;
        int count = 0;
        for (int j = i-window_size + 1; j <= i; j++) {
            if (j >= 0) {
                sum += norm_data_in[j];
                count++;
            }
        }
        fprintf(fp, "%.6f\n", sum / count);
    }
    fclose(fp);
}
 // Compute FFT magnitude and save to file
void compute_fft(const char* out_file) {
    FILE *fp = fopen(out_file, "w");
    if (!fp) {
        perror("FFT file error");
        exit(EXIT_FAILURE);
    }
    for (int k = 0; k < num_samples / 2; k++) {
        double re = 0, im = 0;
        for (int t = 0; t < num_samples; t++) {
            double angle = 2 * PI * t * k / num_samples;
            re += norm_data_in[t] * cos(angle);
            im-= norm_data_in[t] * sin(angle);
        }
        double mag = sqrt(re * re + im * im) / num_samples;
        fprintf(fp, "%d %.6f\n", k, mag);
    }
    fclose(fp);
}
void generate_window(double *w, int N, const char *type) {
    for (int n = 0; n < N; n++) {
        if (strcmp(type, "rect") == 0)
        w[n] = 1.0;
        else if (strcmp(type, "tri") == 0)
        w[n] = 1.0-fabs((n-(N-1) / 2.0) / ((N-1) / 2.0));
        else if (strcmp(type, "hamming") == 0)
        w[n] = 0.54-0.46 * cos((2 * PI * n) / (N-1));
        else if (strcmp(type, "hanning") == 0)
        w[n] = 0.5-0.5 * cos((2 * PI * n) / (N-1));
        else if (strcmp(type, "blackman") == 0)
        w[n] = 0.42-0.5 * cos((2 * PI * n) / (N-1)) + 0.08 * cos((4 * PI * n) / (N-1));
    }
}
 // Step 4: Apply FIR filter and save output
void fir_filter(double *window, int N, const char *output_file) {
    FILE *fp = fopen(output_file, "w");
    FILE *weights_out_fp = fopen("weights_out.txt", "w");
    if (!fp || !weights_out_fp) {
        perror("FIR weight output file open failed");
        exit(EXIT_FAILURE);
    }
    double norm = 0;
    for (int i = 0; i < N; i++) norm += window[i];
    for (int i = 0; i < num_samples; i++) {
        double sum = 0;
        for (int j = 0; j < N; j++) {
            int k = i-j;
            if (k >= 0)
            sum += norm_data_in[k] * window[j];
        }
        double norm_output = sum / norm;
        double weight_out = (norm_output-ZERO_CAL) / SCALE_CAL;
        fprintf(fp, "%.6f\n", norm_output);
        fprintf(weights_out_fp, "%.6f\n", weight_out);
    }
}
int main() {
    read_and_process_adc("input_data_file_name.txt");
    // Step 1: FFT on normalized data
    compute_fft("fft_output.txt");
    // Apply Moving Average filter
    moving_average(FIR_ORDER, "mov_avg.txt");
    // Step 2: FIR on normalized data and convert result to weights
    double window[FIR_ORDER];
    // FIR filters using different windows
    generate_window(window, FIR_ORDER, "rect");
    fir_filter(window, FIR_ORDER, "fir_rect.txt");
    generate_window(window, FIR_ORDER, "tri");
    fir_filter(window, FIR_ORDER, "fir_tri.txt");
    generate_window(window, FIR_ORDER, "hamming");
    fir_filter(window, FIR_ORDER, "fir_hamming.txt");
    generate_window(window, FIR_ORDER, "hanning");
    fir_filter(window, FIR_ORDER, "fir_hanning.txt");
    generate_window(window, FIR_ORDER, "blackman");
    fir_filter(window, FIR_ORDER, "fir_blackman.txt");
    printf("✅ Normalization, FFT, and FIR filtering complete. All results saved.\n");
    return 0;
}
