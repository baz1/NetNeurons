/*
 * Copyright (c) 2015, Rémi Bazin <bazin.remi@gmail.com>
 * All rights reserved.
 * See LICENSE for licensing details.
 */

/*!
    \class Perceptron
    \inmodule NetNeurons
    \ingroup NetNeurons

    \brief Perceptron is a simple interface for managing multilayer perceptrons.

    If you are looking for something more customizable, yet less efficient,
    you might want to take a look at the Neuron and BrainInterface classes.

    \sa Neuron
    \sa BrainInterface
*/

#include "perceptron.h"

#include <string.h>
#include <math.h>
#include <stdio.h>

/* Let's find a portable way of counting the number of cores */
#ifdef __unix__
 #ifdef __linux__
  #include <unistd.h>
  #define PERCEPTRON_DEFAULT_THREADS() sysconf(_SC_NPROCESSORS_CONF)
 #else
  #if __cplusplus > 199711L
   #include <thread>
   #define PERCEPTRON_DEFAULT_THREADS() std::thread::hardware_concurrency();
  #else
   #include <sys/param.h>
   #ifdef BSD
    #include <sys/types.h>
    #include <sys/sysctl.h>
    int PERCEPTRON_DEFAULT_THREADS()
    {
        int mib[4], numCPU;
        size_t len = sizeof(numCPU);
        mib[0] = CTL_HW;
        mib[1] = HW_AVAILCPU;
        sysctl(mib, 2, &numCPU, &len, NULL, 0);
        if (numCPU < 1)
        {
            mib[1] = HW_NCPU;
            sysctl(mib, 2, &numCPU, &len, NULL, 0);
            if (numCPU < 1)
                numCPU = 1
        }
        return numCPU;
    }
   #else
    #warning Unable to retrieve the available number of threads; defaulting to 2.
    #define PERCEPTRON_DEFAULT_THREADS() 2
   #endif
  #endif
 #endif
#endif

/* C++ Double expansion trick */
#define PERCEPTRON_S(x) #x
#define PERCEPTRON_S_(x) PERCEPTRON_S(x)

#define ERROR(str) fprintf(stderr, __FILE__ " (" PERCEPTRON_S_(__LINE__) "): " str)

/* Qt Debug mode */
#ifdef QT_CORE_LIB
 #undef DEBUG_MODE
 #ifdef QT_NO_DEBUG
  #define DEBUG_MODE 0
 #else
  #define DEBUG_MODE 1
 #endif
#endif

/*!
    Constructs a multilayer perceptron.

    It will have \a nInputs inputs, \a nOutputs and \a nHiddenLayers layers of hidden
    neurons in between, each one containing \a nHiddenSize neurons.
    All of these parameters only accept arguments that are stricly positive.

    \note After a call to this constructor, all the weights are initialized to one.

    \note Complexity is O((\c nInputs + \c nOutputs + \c nHiddenLayers * \c nHiddenSize) * \c nHiddenSize)
    both in time and memory (space needed for the class instance).
*/
Perceptron::Perceptron(int nInputs, int nOutputs, int nHiddenSize, int nHiddenLayers)
    : nInputs(nInputs), nOutputs(nOutputs), nHiddenSize(nHiddenSize), nHiddenLayers(nHiddenLayers), weights(NULL), grad(NULL), main_v_data(NULL)
{
    int productSize;
    double init_weight;
    if ((nInputs <= 0) || (nOutputs <= 0) || (nHiddenSize <= 0) || (nHiddenLayers <= 0))
    {
        ERROR("In Perceptron::Perceptron, the inputs should be strictly greater than 0.");
        return;
    }
    weights = new double*[nHiddenLayers + 1];
    weights[0] = new double[(productSize = (nInputs + 1) * nHiddenSize)];
    init_weight = 1. / (nInputs + 1);
    for (int j = productSize; --j >= 0;)
        weights[0][j] = init_weight;
    productSize = (nHiddenSize + 1) * nHiddenSize;
    init_weight = 1. / (nHiddenSize + 1);
    for (int i = nHiddenLayers; --i;)
    {
        weights[i] = new double[productSize];
        for (int j = productSize; --j >= 0;)
            weights[i][j] = init_weight;
    }
    weights[nHiddenLayers] = new double[(productSize = (nHiddenSize + 1) * nOutputs)];
    for (int j = productSize; --j >= 0;)
        weights[nHiddenLayers][j] = init_weight;
#ifdef __unix__
    threads = NULL;
#endif
}

/*!
    Destructs the multilayer perceptron.

    \note Complexity is O(\c nHiddenLayers).
*/
Perceptron::~Perceptron()
{
#ifdef __unix__
    killThreads();
#endif
    if (main_v_data)
    {
        freeNeurons(main_v_data);
        freeNeurons(main_g_data);
    }
    if (!weights)
        return;
    for (int i = nHiddenLayers + 1; i--;)
        delete[] weights[i];
    delete[] weights;
    if (grad)
    {
        for (int i = nHiddenLayers + 1; i--;)
        {
            delete[] grad[i];
            delete[] learning_rates[i];
            delete[] former_grad[i];
        }
        delete[] grad;
        delete[] learning_rates;
        delete[] former_grad;
    }
}

/*!
    \fn bool Perceptron::hasError() const

    Checks whether or not an error happened during the creation of the perceptron.

    Returns \c true if the constructor failed, \c false otherwise.

    \note It actually returns \c true only if one of the arguments passed to the constructor
    was not stricly positive.

    \note Complexity is O(1).
*/

/*!
    Calculates the output of the multilayer perceptron on the given input values vector \a input.

    \note It is the responsibility of the user to delete the array that is returned.

    \note Complexity is O((\c nInputs + \c nOutputs + \c nHiddenLayers * \c nHiddenSize) * \c nHiddenSize).
*/
double *Perceptron::calculate(double * input) const
{
    if (!weights)
    {
        ERROR("In Perceptron::calculate, the perceptron has errors.");
        return NULL;
    }
    double tmp;
    double *outputs = new double[nHiddenSize];
    double *tmpPtr = (nHiddenLayers > 1) ? (new double[nHiddenSize]) : NULL;
    int hiddenSpace = sizeof(double) * nHiddenSize;
    memset(outputs, 0, hiddenSpace);
    int offset = 0;
    for (int i = 0; i < nInputs; ++i)
    {
        tmp = input[i];
        for (int j = 0; j < nHiddenSize; ++j)
            outputs[j] += weights[0][offset++] * tmp;
    }
    for (int j = 0; j < nHiddenSize; ++j)
        outputs[j] += weights[0][offset++];
    for (int k = 1; k < nHiddenLayers; ++k)
    {
        input = outputs;
        outputs = tmpPtr;
        tmpPtr = input;
        memset(outputs, 0, hiddenSpace);
        offset = 0;
        for (int i = 0; i < nHiddenSize; ++i)
        {
            tmp = tanh(input[i]);
            for (int j = 0; j < nHiddenSize; ++j)
                outputs[j] += weights[k][offset++] * tmp;
        }
        for (int j = 0; j < nHiddenSize; ++j)
            outputs[j] += weights[k][offset++];
    }
    if (nHiddenLayers > 1)
        delete[] input;
    input = outputs;
    outputs = new double[nOutputs];
    memset(outputs, 0, sizeof(double) * nOutputs);
    offset = 0;
    for (int i = 0; i < nHiddenSize; ++i)
    {
        tmp = tanh(input[i]);
        for (int j = 0; j < nOutputs; ++j)
            outputs[j] += weights[nHiddenLayers][offset++] * tmp;
    }
    for (int j = 0; j < nOutputs; ++j)
        outputs[j] += weights[nHiddenLayers][offset++];
    return outputs;
}

/*!
    Launches a number of threads for the training to be multithreaded.

    The parameter \a n_threads lets you specify the number of threads you want to use
    in order to compute each training step. If \a n_threads is less than 1,
    the number of available CPU is used.

    \note This function only works on UNIX (else, it does nothing).
*/
void Perceptron::multithreadedTrain(int n_threads)
{
#ifdef __unix__
    killThreads();
    if (!weights)
    {
        ERROR("In Perceptron::multithreadedTrain, the perceptron has errors.");
        return;
    }
    if (n_threads <= 0)
        n_threads = PERCEPTRON_DEFAULT_THREADS();
    if (n_threads <= 1)
        return;
#if DEBUG_MODE
    fprintf(stderr, "Perceptron::multithreadedTrain: Using %d threads\n", n_threads);
#endif
    pthread_mutex_init(&grad_protect, NULL);
    pthread_cond_init(&wait_end, NULL);
    pthread_mutex_init(&cond_mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&nextc, NULL);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    t_exit = false;
    t_inputs = 0;
    t_outputs = 0;
    threads = new pthread_t[(t_count = n_threads)];
    while (--n_threads >= 0)
        pthread_create(&threads[n_threads], &attr, thread_run, (void*) this);
    pthread_attr_destroy(&attr);
#endif
}

/*!
    Kills all the threads that have been created with multithreadedTrain.

    This function is automatically called in the destructor, and is equivalent
    to calling multithreadedTrain(1).

    \sa multithreadedTrain()
*/
void Perceptron::killThreads()
{
#ifdef __unix__
    if (!threads)
        return;
    pthread_mutex_lock(&cond_mutex);
    t_exit = true;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&cond_mutex);
    for (int i = t_count - 1; i >= 0; --i)
        pthread_join(threads[i], NULL);
    pthread_mutex_destroy(&cond_mutex);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&nextc);
    pthread_mutex_destroy(&grad_protect);
    pthread_cond_destroy(&wait_end);
    delete[] threads;
    threads = NULL;
#endif
}

/*!
    Trains the multilayer perceptron with the given data.

    Returns the error of the weights \underline before the training step.

    \a size is the number of items in \a inputs and \a outputs.
    Each of these items are input and output values.

    \warning This function only does one step of training;
    you are expected to call it several times in order to obtain
    a trained perceptron.

    \note The returned value may be useful to retrieve the learning progress
    over time.

    \note This function might use several threads if you called the function
    multithreadedTrain beforehand.

    \sa multithreadedTrain()
*/
double Perceptron::train(int size, double **inputs, double **outputs)
{
    if (!weights)
    {
        ERROR("In Perceptron::train, the perceptron has errors.");
        return -1;
    }
    int productSize;
    if (!grad)
    {
        int tmp = nHiddenLayers + 1;
        grad = new double*[tmp];
        learning_rates = new double*[tmp];
        former_grad = new double*[tmp];
        grad[0] = new double[(productSize = (nInputs + 1) * nHiddenSize)];
        learning_rates[0] = new double[productSize];
        former_grad[0] = new double[productSize];
        tmp = sizeof(double) * productSize;
        memset(grad[0], 0, tmp);
        for (int j = productSize - 1; j >= 0; --j)
            learning_rates[0][j] = PERCEPTRON_DEFAULT_LEARNING_RATE;
        memset(former_grad[0], 0, tmp);
        productSize = (nHiddenSize + 1) * nHiddenSize;
        tmp = sizeof(double) * productSize;
        for (int i = nHiddenLayers; --i;)
        {
            grad[i] = new double[productSize];
            learning_rates[i] = new double[productSize];
            former_grad[i] = new double[productSize];
            memset(grad[i], 0, tmp);
            for (int j = productSize - 1; j >= 0; --j)
                learning_rates[i][j] = PERCEPTRON_DEFAULT_LEARNING_RATE;
            memset(former_grad[i], 0, tmp);
        }
        grad[nHiddenLayers] = new double[(productSize = (nHiddenSize + 1) * nOutputs)];
        learning_rates[nHiddenLayers] = new double[productSize];
        former_grad[nHiddenLayers] = new double[productSize];
        tmp = sizeof(double) * productSize;
        memset(grad[nHiddenLayers], 0, tmp);
        for (int j = productSize - 1; j >= 0; --j)
            learning_rates[nHiddenLayers][j] = PERCEPTRON_DEFAULT_LEARNING_RATE;
        memset(former_grad[nHiddenLayers], 0, tmp);
    }
    err = 0;
#ifdef __unix__
    if (threads)
    {
        t_left = size;
        pthread_mutex_lock(&cond_mutex);
        while (size--)
        {
            t_inputs = inputs[size];
            t_outputs = outputs[size];
            pthread_cond_signal(&cond);
            do {
                pthread_cond_wait(&nextc, &cond_mutex);
            } while (t_inputs || t_outputs);
        }
        pthread_mutex_unlock(&cond_mutex);
        pthread_mutex_lock(&grad_protect);
        while (true)
        {
            if (!t_left)
                break;
            pthread_cond_wait(&wait_end, &grad_protect);
        }
        pthread_mutex_unlock(&grad_protect);
    } else {
#else
    {
#endif
        if (!main_v_data)
        {
            main_v_data = allocNeurons();
            main_g_data = allocNeurons();
        }
        while (size--)
            trainSingleInput(inputs[size], outputs[size], main_v_data, main_g_data);
    }
    for (int j = (nInputs + 1) * nHiddenSize - 1; j >= 0; --j)
        trainSingleWeight(0, j);
    productSize = (nHiddenSize + 1) * nHiddenSize;
    for (int i = nHiddenLayers; --i;)
    {
        for (int j = productSize - 1; j >= 0; --j)
            trainSingleWeight(i, j);
    }
    for (int j = (nHiddenSize + 1) * nOutputs - 1; j >= 0; --j)
        trainSingleWeight(nHiddenLayers, j);
    return err;
}

#ifdef __unix__

void *Perceptron::thread_run(void *obj)
{
    Perceptron *my_this = reinterpret_cast<Perceptron*>(obj);
    {
        /* Small thread check to avoid user tampering */
        pthread_t self = pthread_self();
        for (int i = my_this->t_count - 1; true; --i)
        {
            if (i < 0)
                return NULL;
            if (pthread_equal(self, my_this->threads[i]))
                break;
        }
    }
    double *my_input, *my_output;
    double **my_v_data, **my_g_data;
    my_v_data = my_this->allocNeurons();
    my_g_data = my_this->allocNeurons();
    while (true)
    {
        pthread_mutex_lock(&my_this->cond_mutex);
        while (true)
        {
            if (my_this->t_exit)
            {
                pthread_mutex_unlock(&my_this->cond_mutex);
                my_this->freeNeurons(my_v_data);
                my_this->freeNeurons(my_g_data);
                pthread_exit(NULL);
            }
            if (my_this->t_inputs && my_this->t_outputs)
                break;
            pthread_cond_wait(&my_this->cond, &my_this->cond_mutex);
        }
        my_input = (double*) my_this->t_inputs;
        my_output = (double*) my_this->t_outputs;
        my_this->t_inputs = 0;
        my_this->t_outputs = 0;
        pthread_cond_signal(&my_this->nextc);
        pthread_mutex_unlock(&my_this->cond_mutex);
        my_this->trainSingleInput(my_input, my_output, my_v_data, my_g_data);
    }
}

#endif

inline double sqr(double x)
{
    return x * x;
}

void Perceptron::trainSingleInput(double *input, double *output, double **v_data, double **g_data)
{
    double my_err = 0, tmp;
    double *aptr, *wptr, *ptr3;
    int hiddenSpace = sizeof(double) * nHiddenSize;
    memset(v_data[0], 0, hiddenSpace);
    int offset = 0;
    aptr = v_data[0];
    wptr = weights[0];
    for (int i = 0; i < nInputs; ++i)
    {
        tmp = input[i];
        for (int j = 0; j < nHiddenSize; ++j)
            aptr[j] += wptr[offset++] * tmp;
    }
    for (int j = 0; j < nHiddenSize; ++j)
        aptr[j] = tanh(aptr[j] + wptr[offset++]);
    for (int k = 1; k < nHiddenLayers; ++k)
    {
        aptr = v_data[k];
        wptr = weights[k];
        memset(v_data[k], 0, hiddenSpace);
        offset = 0;
        for (int i = 0; i < nHiddenSize; ++i)
        {
            tmp = v_data[k - 1][i];
            for (int j = 0; j < nHiddenSize; ++j)
                aptr[j] += wptr[offset++] * tmp;
        }
        for (int j = 0; j < nHiddenSize; ++j)
            aptr[j] = tanh(aptr[j] + wptr[offset++]);
    }
    memset(v_data[nHiddenLayers], 0, sizeof(double) * nOutputs);
    offset = 0;
    aptr = v_data[nHiddenLayers];
    wptr = weights[nHiddenLayers];
    for (int i = 0; i < nHiddenSize; ++i)
    {
        tmp = v_data[nHiddenLayers - 1][i];
        for (int j = 0; j < nOutputs; ++j)
            aptr[j] += wptr[offset++] * tmp;
    }
    for (int j = 0; j < nOutputs; ++j)
        my_err += sqr(g_data[nHiddenLayers][j] = (aptr[j] + wptr[offset++]) - output[j]);
    // Note: (aptr[j] += wptr[offset++]) instead of just + if we want the real final value
    aptr = g_data[nHiddenLayers];
    offset = 0;
    for (int i = 0; i < nHiddenSize; ++i)
    {
        tmp = 0;
        for (int j = 0; j < nOutputs; ++j)
            tmp += aptr[j] * wptr[offset++];
        tmp *= 1. - sqr(v_data[nHiddenLayers - 1][i]);
        g_data[nHiddenLayers - 1][i] = tmp;
    }
    for (int k = nHiddenLayers - 2; k >= 0; --k)
    {
        aptr = g_data[k + 1];
        wptr = weights[k + 1];
        offset = 0;
        for (int i = 0; i < nHiddenSize; ++i)
        {
            tmp = 0;
            for (int j = 0; j < nHiddenSize; ++j)
                tmp += aptr[j] * wptr[offset++];
            tmp *= 1. - sqr(v_data[k][i]);
            g_data[k][i] = tmp;
        }
    }
#ifdef __unix__
    if (threads)
        pthread_mutex_lock(&grad_protect);
#endif
    aptr = grad[nHiddenLayers];
    wptr = v_data[nHiddenLayers - 1];
    ptr3 = g_data[nHiddenLayers];
    offset = 0;
    for (int i = 0; i < nHiddenSize; ++i)
    {
        for (int j = 0; j < nOutputs; ++j)
            aptr[offset++] += ptr3[j] * wptr[i];
    }
    for (int j = 0; j < nOutputs; ++j)
        aptr[offset++] += ptr3[j];
    for (int k = nHiddenLayers; --k > 0;)
    {
        aptr = grad[k];
        wptr = v_data[k - 1];
        ptr3 = g_data[k];
        offset = 0;
        for (int i = 0; i < nHiddenSize; ++i)
        {
            for (int j = 0; j < nHiddenSize; ++j)
                aptr[offset++] += ptr3[j] * wptr[i];
        }
        for (int j = 0; j < nHiddenSize; ++j)
            aptr[offset++] += ptr3[j];
    }
    aptr = grad[0];
    wptr = input;
    ptr3 = g_data[0];
    offset = 0;
    for (int i = 0; i < nInputs; ++i)
    {
        for (int j = 0; j < nHiddenSize; ++j)
            aptr[offset++] += ptr3[j] * wptr[i];
    }
    for (int j = 0; j < nHiddenSize; ++j)
        aptr[offset++] += ptr3[j];
    err += my_err;
#ifdef __unix__
    if (threads)
    {
        --t_left;
        pthread_cond_signal(&wait_end);
        pthread_mutex_unlock(&grad_protect);
    }
#endif
}

double **Perceptron::allocNeurons()
{
    double **result = new double*[nHiddenLayers + 1];
    result[nHiddenLayers] = new double[nOutputs];
    for (int i = nHiddenLayers - 1; i >= 0; --i)
        result[i] = new double[nHiddenSize];
    return result;
}

void Perceptron::freeNeurons(double **ptr)
{
    for (int i = nHiddenLayers; i >= 0; --i)
        delete[] ptr[i];
    delete[] ptr;
}
