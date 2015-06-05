/*
 * Copyright (c) 2015, Rémi Bazin <bazin.remi@gmail.com>
 * All rights reserved.
 * See LICENSE for licensing details.
 */

#ifndef NEURON_H
#define NEURON_H

#include <QList>

#define NEURON_DEFAULT_LEARNING_RATE 0.1
#define NEURON_INCREASE_LEARNING 1.5
#define NEURON_DECREASE_LEARNING 0.4

#ifndef NEURON_ENABLE_LEARNING
#define NEURON_ENABLE_LEARNING 1
#endif

typedef double (*NEURON_FUN) (double);

class BrainInterface;

class Neuron
{
    friend class BrainInterface;
private:
    struct Connection
    {
        Neuron *source;
        double weight;
#if NEURON_ENABLE_LEARNING
        double e, learning_rate, former_e;
#endif
    };
public:
    Neuron(NEURON_FUN activ = NULL, NEURON_FUN deriv = NULL);
    void connectTo(Neuron *otherNeuron, double initialWeight = 1.);
private: /* Accessed from BrainInterface */
    void setValue(double value, bool step = true);
    double getValue(bool step = true);
    void brainDelete(BrainInterface *brain);
#if NEURON_ENABLE_LEARNING
    void train(double expectedOutput);
    void learn();
private:
    double learnMistakes(double influence);
#endif
public:
    static double linear_activ(double x);
    static double linear_deriv(double ax);
    static double exp_activ(double x);
    static double exp_deriv(double ax);
    static NEURON_FUN tanh_activ;
    static double tanh_deriv(double ax);
private:
    NEURON_FUN activ;
    QList<Connection> backwardConnections;
    double a;
    bool a_step;
#if NEURON_ENABLE_LEARNING
    NEURON_FUN deriv;
    int connectedTo, waitingFor;
    double sum;
#endif
};

class BrainInterface
{
    friend class Neuron;
public:
    BrainInterface(QList<Neuron*> inputNeurons, QList<Neuron*> outputNeurons);
    QList<double> run(QList<double> inputValues) const;
    void train(QList<double> inputValues, QList<double> outputValues);
    double learn();
    void deleteBrain();
private: /* Accessed from Neuron */
    void deleteLater(Neuron *neuron);
private:
    QList<Neuron*> inputNeurons, outputNeurons;
    mutable bool currentStep;
    Neuron *previous;
#if NEURON_ENABLE_LEARNING
    double error;
#endif
};

#endif // NEURON_H
