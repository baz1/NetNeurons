/*
 * Copyright (c) 2015, Rémi Bazin <bazin.remi@gmail.com>
 * All rights reserved.
 * See LICENSE for licensing details.
 */

#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#define PERCEPTRON_DEFAULT_LEARNING_RATE 0.1
#define PERCEPTRON_INCREASE_LEARNING 1.5
#define PERCEPTRON_DECREASE_LEARNING 0.4

/* pthread support needed for multithreading. */
#ifdef __unix__
 #include <pthread.h>
#else
 #warning This library does not work on non-unix OS.
#endif

#define DEBUG_MODE 0

class Perceptron
{
public:
    Perceptron(int nInputs, int nOutputs, int nHiddenSize, int nHiddenLayers);
    ~Perceptron();
    inline bool hasError() const;
    double *calculate(double *input) const;
    void multithreadedTrain(int n_threads = 0);
    void killThreads();
    double train(int size, double **inputs, double **outputs);
private:
#ifdef __unix__
    static void *thread_run(void *obj);
#endif
    inline void trainSingleWeight(const int &i1, const int &i2);
    void trainSingleInput(double *input, double *output, double **v_data, double **g_data);
    double **allocNeurons();
    void freeNeurons(double **ptr);
private:
    int nInputs, nOutputs, nHiddenSize, nHiddenLayers;
    double **weights;
    /* weights: First index is layer interval, second index is (source * nDestination + destination) */
    /* The last source is the bias */
    double **grad, **learning_rates, **former_grad;
#ifdef __unix__
    pthread_t *threads;
    int t_count;
    volatile int t_left;
    pthread_mutex_t grad_protect;
    pthread_cond_t cond, nextc, wait_end;
    pthread_mutex_t cond_mutex;
    volatile bool t_exit;
#endif
    double err, **main_v_data, **main_g_data;
    volatile double *t_inputs, *t_outputs;
};

inline bool Perceptron::hasError() const
{
    return !weights;
}

inline void Perceptron::trainSingleWeight(const int &i1, const int &i2)
{
    double g = grad[i1][i2];
    double tmp = g * former_grad[i1][i2];
    if (tmp > 0)
    {
        tmp = (learning_rates[i1][i2] *= PERCEPTRON_INCREASE_LEARNING);
    } else if (tmp < 0)
    {
        tmp = (learning_rates[i1][i2] *= PERCEPTRON_DECREASE_LEARNING);
    } else {
        tmp = learning_rates[i1][i2];
    }
    weights[i1][i2] -= tmp * g;
    former_grad[i1][i2] = g;
    grad[i1][i2] = 0;
}

#endif // PERCEPTRON_H
