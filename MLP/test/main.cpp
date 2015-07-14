#include <QtCore>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>

#include "NetNeurons/neuron.h"
#include "NetNeurons/perceptron.h"

#define HIDDEN_SIZE 400
#define HIDDEN_LAYERS 3
#define LEARNING_SIZE 100

static volatile bool endLoop = false;

void test_fn(double * const inputs, double *outputs)
{
    outputs[0] = (inputs[0] - inputs[1]) * 12;
    outputs[1] = qMax(inputs[2] * 12, inputs[3] * 12) + (inputs[0] - inputs[1]);
    outputs[0] += ((double) (rand() % 100) - 50.) / 2400.;
    outputs[1] += ((double) (rand() % 100) - 50.) / 2400.;
}

double mfrand()
{
    return ((double) (rand() & 0x3FF)) / 0x400;
}

void intSignalHandler(int sig)
{
    Q_UNUSED(sig)
    endLoop = true;
    printf("SigInt received.\n");
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    struct sigaction sigInt;
    sigInt.sa_handler = intSignalHandler;
    sigemptyset(&sigInt.sa_mask);
    sigaction(SIGINT, &sigInt, NULL);
    srand(time(NULL));
    printf("Simplified network initialization...\n");
    fflush(stdout);
    Perceptron *perceptron = new Perceptron(4, 2, HIDDEN_SIZE, HIDDEN_LAYERS);
    printf("Simplified threaded network initialization...\n");
    fflush(stdout);
    Perceptron *tperceptron = new Perceptron(4, 2, HIDDEN_SIZE, HIDDEN_LAYERS);
    printf("Custom network initialization...\n");
    fflush(stdout);
    QList<Neuron*> inputNeurons, stepFrom, stepTo, outputNeurons;
    inputNeurons.reserve(5);
    while (inputNeurons.length() < 5)
        inputNeurons.append(new Neuron());
    outputNeurons.reserve(2);
    while (outputNeurons.length() < 2)
        outputNeurons.append(new Neuron());
    stepFrom = inputNeurons.mid(0, 4);
    double init_weight = 1. / 5.;
    for (int i = HIDDEN_LAYERS; --i >= 0;)
    {
        stepTo.reserve(HIDDEN_SIZE);
        while (stepTo.length() < HIDDEN_SIZE)
            stepTo.append(new Neuron(Neuron::tanh_activ, Neuron::tanh_deriv));
        foreach (Neuron *to, stepTo)
        {
            foreach (Neuron *from, stepFrom)
                from->connectTo(to, init_weight);
            inputNeurons.last()->connectTo(to, init_weight);
        }
        stepFrom = stepTo;
        init_weight = 1. / (HIDDEN_SIZE+1);
    }
    stepTo.clear();
    foreach (Neuron *to, outputNeurons)
    {
        foreach (Neuron *from, stepFrom)
            from->connectTo(to, init_weight);
        inputNeurons.last()->connectTo(to, init_weight);
    }
    stepFrom.clear();
    BrainInterface *interface = new BrainInterface(inputNeurons, outputNeurons);
    printf("Generating learning data...\n");
    fflush(stdout);
    double **inputs, **outputs;
    int size = LEARNING_SIZE;
    inputs = new double*[size];
    outputs = new double*[size];
    for (int i = size; --i >= 0;)
    {
        inputs[i] = new double[4];
        outputs[i] = new double[2];
        inputs[i][3] = mfrand();
        inputs[i][2] = mfrand();
        inputs[i][1] = mfrand() / 2.;
        inputs[i][0] = mfrand();
        test_fn(inputs[i], outputs[i]);
    }
    printf("Learning: (Press Ctrl+C to stop)\n");
    fflush(stdout);
    tperceptron->multithreadedTrain();
    int step = 0;
    while (!endLoop)
    {
        QList<double> inputValues, outputValues;
        printf("Learning step %d:\n", ++step);
        fflush(stdout);
        QTime timer;
        timer.start();
        double result = perceptron->train(size, inputs, outputs);
        int t = timer.elapsed();
        printf("  Perceptron: %d ms (%lf)\n", t, result);
        fflush(stdout);
        timer.start();
        result = tperceptron->train(size, inputs, outputs);
        t = timer.elapsed();
        printf("  Threaded Perceptron: %d ms (%lf)\n", t, result);
        fflush(stdout);
        timer.start();
        for (int i = size; --i >= 0;)
        {
            inputValues.reserve(5);
            for (int j = 3; j >= 0; --j)
                inputValues.append(inputs[i][j]);
            inputValues.append(1);
            outputValues.append(outputs[i][0]);
            outputValues.append(outputs[i][1]);
            interface->train(inputValues, outputValues);
            inputValues.clear();
            outputValues.clear();
        }
        result = interface->learn();
        t = timer.elapsed();
        printf("  BrainInterface: %d ms (%lf)\n", t, result);
        fflush(stdout);
    }
    printf("Simplified network termination...\n");
    fflush(stdout);
    delete perceptron;
    printf("Custom network termination...\n");
    fflush(stdout);
    interface->deleteBrain();
    delete interface;
    printf("Done.\n");
    fflush(stdout);
    return 0;
}
