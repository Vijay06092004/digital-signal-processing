# 🧠 Advanced Signal Filtering Techniques

This repository demonstrates and compares multiple signal filtering techniques applied to time-series data (e.g., sensor data from ADCs). These filters include traditional FIR window-based filters, frequency-domain designs, and adaptive filters like Kalman and Wavelet filters.

---

## 📌 Table of Contents

- [Introduction](#introduction)
- [Filter Overview](#filter-overview)
- [Detailed Filtering Methods](#detailed-filtering-methods)
  - [1. Rectangular Window](#1-rectangular-window)
  - [2. Triangular Window](#2-triangular-window)
  - [3. Hamming Window](#3-hamming-window)
  - [4. Hanning (Hann) Window](#4-hanning-hann-window)
  - [5. Blackman Window](#5-blackman-window)
  - [6. Kaiser Window](#6-kaiser-window)
  - [7. Frequency Sampling Method](#7-frequency-sampling-method)
  - [8. Least Squares Method](#8-least-squares-method)
  - [9. Parks-McClellan (Equiripple) Method](#9-parks-mcclellan-equiripple-method)
  - [10. Wavelet Filtering](#10-wavelet-filtering)
  - [11. Kalman Filtering](#11-kalman-filtering)
- [Comparison Table](#comparison-table)
- [How to Use](#how-to-use)
- [References](#references)
- [License](#license)

---

## 📖 Introduction

Filtering is a fundamental tool in digital signal processing (DSP), used for noise removal, smoothing, or enhancing specific features in signals. This repository provides implementations of both classical and advanced filtering methods in **C/MATLAB**, suitable for embedded and desktop systems.

---

## 🧮 Filter Overview

| Category               | Methods                                                                 |
|------------------------|-------------------------------------------------------------------------|
| FIR Window Filters     | Rectangular, Triangular, Hamming, Hanning, Blackman, Kaiser             |
| FIR Design Techniques  | Frequency Sampling, Least Squares, Parks-McClellan                     |
| Adaptive / Recursive   | Wavelet Filtering, Kalman Filtering                                     |

---

## 🧩 Detailed Filtering Methods

### 1. Rectangular Window

- **Type**: FIR (Window method)
- **Formula**: `w[n] = 1` for all `n`
- **Use Case**: Simple applications where sharp transitions are less critical
- **Advantages**: Easy to implement
- **Disadvantages**: High side lobe levels → poor frequency resolution

---

### 2. Triangular Window

- **Formula**: `w[n] = 1 - |(n - (N - 1)/2)| / ((N - 1)/2)`
- **Advantages**: Lower spectral leakage than Rectangular
- **Disadvantages**: Still moderate side lobes

---

### 3. Hamming Window

- **Formula**: `w[n] = 0.54 - 0.46 * cos(2πn / (N - 1))`
- **Advantages**: Good balance between main lobe width and side lobe attenuation (~43 dB)
- **Disadvantages**: Slightly wider transition than Rectangular

---

### 4. Hanning (Hann) Window

- **Formula**: `w[n] = 0.5 - 0.5 * cos(2πn / (N - 1))`
- **Advantages**: Smooth taper, reduces leakage
- **Disadvantages**: Weaker suppression compared to Hamming

---

### 5. Blackman Window

- **Formula**:  
  `w[n] = 0.42 - 0.5*cos(2πn/(N-1)) + 0.08*cos(4πn/(N-1))`
- **Advantages**: Excellent side lobe suppression (~58 dB)
- **Disadvantages**: Wider main lobe → poorer frequency resolution

---

### 6. Kaiser Window

- **Formula**:  
  `w[n] = I₀(β * √(1 - ((2n / (N-1)) - 1)²)) / I₀(β)`  
  where `I₀` is the zeroth-order modified Bessel function
- **Advantages**: Adjustable β allows control over trade-off between main lobe width and side lobe level
- **Disadvantages**: Slightly complex to compute

---

### 7. Frequency Sampling Method

- **Concept**: Define desired frequency response at discrete points and use IDFT to compute impulse response
- **Advantages**: Direct frequency-domain control
- **Disadvantages**: Can result in non-ideal time-domain behavior

---

### 8. Least Squares Method

- **Goal**: Minimize mean square error between desired and actual frequency response
- **Advantages**: Smooth approximation, good for audio/speech
- **Disadvantages**: May not meet strict magnitude constraints

---

### 9. Parks-McClellan (Equiripple) Method

- **Algorithm**: Remez Exchange (Minimax optimization)
- **Goal**: Minimize maximum deviation in the pass/stop bands
- **Advantages**: Sharp transition band, optimal ripple control
- **Disadvantages**: More computationally intensive to design

---

### 10. Wavelet Filtering

- **Concept**: Multi-resolution decomposition of signal
- **Approach**: Threshold wavelet coefficients, then reconstruct
- **Advantages**: Excellent for non-stationary signals, denoising
- **Disadvantages**: Requires choice of wavelet family and threshold level

---

### 11. Kalman Filtering

- **Type**: Recursive, optimal estimator
- **Model**:
