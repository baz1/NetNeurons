/*
 * Copyright (c) 2015, Rémi Bazin <bazin.remi@gmail.com>
 * All rights reserved.
 * See LICENSE for licensing details.
 */

/*!
    \class Neuron
    \inmodule NetNeurons
    \ingroup NetNeurons

    \brief This class allows you to construct every mulilayer perceptron you want.

    It is less efficient than the Perceptron, but far more customizable.
    The BrainInterface class is here to help you manage your network of neurons once
    is has been constructed.

    \sa Perceptron
    \sa BrainInterface
*/

#include "neuron.h"

#include <math.h>

/*!
    \typedef NEURON_FUN
    \relates Neuron

    Function pointer type working on double variables.
*/

/*!
    Constructs a neuron.

    \a activ shall be the activation function for the neuron.
    If we write g its inverse function and h its derivative, \a deriv shall be (h o g).

    If at least one is not provided, \a activ is supposed to be the identity and \a deriv the constant 1.
*/
Neuron::Neuron(NEURON_FUN activ, NEURON_FUN deriv)
    : activ(activ), a(0), a_step(false)
#if NEURON_ENABLE_LEARNING
    , deriv(deriv), connectedTo(0), waitingFor(0), sum(0)
#endif
{
    if (((bool) activ) ^ ((bool) deriv))
        activ = deriv = 0;
}

/*!
    Connects the neuron to another neuron \a otherNeuron.

    You can specify an initial weight \a initialWeight that defaults to 1.
*/
void Neuron::connectTo(Neuron *otherNeuron, double initialWeight)
{
    Connection to_add;
    to_add.source = this;
    to_add.weight = initialWeight;
#if NEURON_ENABLE_LEARNING
    to_add.e = 0;
    to_add.learning_rate = NEURON_DEFAULT_LEARNING_RATE;
    to_add.former_e = 0;
    waitingFor = ++connectedTo;
#endif
    otherNeuron->backwardConnections.append(to_add);
}

void Neuron::setValue(double value, bool step)
{
    a = value;
    a_step = step;
}

double Neuron::getValue(bool step)
{
    if (a_step == step)
        return a;
    a = 0;
    a_step = step;
    for (int i = backwardConnections.length() - 1; i >= 0; --i)
        a += backwardConnections.at(i).weight * backwardConnections.at(i).source->getValue(step);
    if (activ)
        a = activ(a);
    return a;
}

void Neuron::brainDelete(BrainInterface *brain)
{
    if (--waitingFor != 0)
        return;
    for (int i = backwardConnections.length() - 1; i >= 0; --i)
        backwardConnections.at(i).source->brainDelete(brain);
    brain->deleteLater(this);
}

#if NEURON_ENABLE_LEARNING

void Neuron::train(double expectedOutput)
{
    double myInfluence = (a - expectedOutput) * (deriv ? deriv(a) : 1.);
    for (int i = backwardConnections.length() - 1; i >= 0; --i)
        backwardConnections[i].e += backwardConnections.at(i).source->learnMistakes(myInfluence * backwardConnections.at(i).weight) * myInfluence;
}

void Neuron::learn()
{
    if (--waitingFor > 0)
        return;
    waitingFor = connectedTo;
    double e, tmp;
    for (int i = backwardConnections.length() - 1; i >= 0; --i)
    {
        e = backwardConnections.at(i).e;
        tmp = e * backwardConnections.at(i).former_e;
        if (tmp > 0)
        {
            tmp = (backwardConnections[i].learning_rate *= NEURON_INCREASE_LEARNING);
        } else if (tmp < 0)
        {
            tmp = (backwardConnections[i].learning_rate *= NEURON_DECREASE_LEARNING);
        } else {
            tmp = backwardConnections.at(i).learning_rate;
        }
        backwardConnections[i].weight -= tmp * e;
        backwardConnections[i].former_e = e;
        backwardConnections[i].e = 0;
        backwardConnections.at(i).source->learn();
    }
}

double Neuron::learnMistakes(double influence)
{
    sum += influence;
    if (--waitingFor != 0)
        return a;
    waitingFor = connectedTo;
    double myInfluence = sum * (deriv ? deriv(a) : 1.);
    sum = 0;
    for (int i = backwardConnections.length() - 1; i >= 0; --i)
        backwardConnections[i].e += backwardConnections.at(i).source->learnMistakes(myInfluence * backwardConnections.at(i).weight) * myInfluence;
    return a;
}

#endif

/*!
    Linear activ function (parameter \a x).
*/
double Neuron::linear_activ(double x)
{
    return x;
}

/*!
    Linear deriv function (parameter \a ax).
*/
double Neuron::linear_deriv(double ax)
{
    Q_UNUSED(ax)
    return 1.;
}

/*!
    Exponential activ function (parameter \a x).
*/
double Neuron::exp_activ(double x)
{
    return 1. / (1. + exp(-x));
}

/*!
    Exponential deriv function (parameter \a ax).
*/
double Neuron::exp_deriv(double ax)
{
    return (ax < 1.) ? (ax * ax * ax / (1. - ax)) : 0.;
}

/*!
    \variable Neuron::tanh_activ
    Tanh activ function (parameter \a x).
*/
NEURON_FUN Neuron::tanh_activ = tanh;

/*!
    Tanh deriv function (parameter \a ax).
*/
double Neuron::tanh_deriv(double ax)
{
    return 1. - ax * ax;
}

/*!
    \class BrainInterface
    \inmodule NetNeurons
    \ingroup NetNeurons

    \brief This class allows you to manage your neurons more easily.

    \sa Neuron
*/

/*!
    Constructs a new brain interface based on an already existing network of neurons.

    \a inputNeurons is the list of input neurons and \a outputNeurons the output neurons
    of the perceptron corresponding to the - supposed already built - network of neurons.
*/
BrainInterface::BrainInterface(QList<Neuron *> inputNeurons, QList<Neuron *> outputNeurons)
    : inputNeurons(inputNeurons), outputNeurons(outputNeurons), currentStep(false), previous(NULL)
#if NEURON_ENABLE_LEARNING
    , error(0)
#endif
{
}

/*!
    Runs the network on the given \a inputValues and returns the output values.
*/
QList<double> BrainInterface::run(QList<double> inputValues) const
{
    if (inputValues.length() != inputNeurons.length())
        return QList<double>();
    currentStep = !currentStep;
    for (int i = inputNeurons.length() - 1; i >= 0; --i)
        inputNeurons.at(i)->setValue(inputValues.at(i), currentStep);
    QList<double> result;
    result.reserve(outputNeurons.length());
    for (int i = 0; i < outputNeurons.length(); ++i)
        result.append(outputNeurons.at(i)->getValue(currentStep));
    return result;
}

inline double sqr(double x)
{
    return x * x;
}

/*!
    Trains the network on a couple of \a inputValues and \a outputValues.

    \note This does not change the weights at all until you call the function learn.

    \note The macro value \c NEURON_ENABLE_LEARNING needs to be true (default value)
    for this function to operate.

    \sa BrainInterface::learn()
*/
void BrainInterface::train(QList<double> inputValues, QList<double> outputValues)
{
#if NEURON_ENABLE_LEARNING
    if ((inputValues.length() != inputNeurons.length()) || (outputValues.length() != outputNeurons.length()))
        return;
    currentStep = !currentStep;
    for (int i = inputNeurons.length() - 1; i >= 0; --i)
        inputNeurons.at(i)->setValue(inputValues.at(i), currentStep);
    for (int i = outputNeurons.length() - 1; i >= 0; --i)
        error += sqr(outputNeurons.at(i)->getValue(currentStep) - outputValues.at(i));
    for (int i = outputNeurons.length() - 1; i >= 0; --i)
        outputNeurons.at(i)->train(outputValues.at(i));
#endif
}

/*!
    Computes a learning step based on the previous calls to the train function.

    \note The macro value \c NEURON_ENABLE_LEARNING needs to be true (default value)
    for this function to operate.

    \sa BrainInterface::train(QList<double> inputValues, QList<double> outputValues)
*/
double BrainInterface::learn()
{
#if NEURON_ENABLE_LEARNING
    for (int i = outputNeurons.length() - 1; i >= 0; --i)
        outputNeurons.at(i)->learn();
    double result = error;
    error = 0;
    return result;
#else
    return -1;
#endif
}

/*!
    Deletes the brain.

    Even though the neurons should have been manually created before the creation of this
    BrainInterface object, you can delete all the network at once by using this function.
    All that is left after that is to delete the BrainInterface.

    \warning You need to create the neurons one by one (i.e. one new by neuron) to be able to
    correctly use this function.
*/
void BrainInterface::deleteBrain()
{
    for (int i = outputNeurons.length() - 1; i >= 0; --i)
        outputNeurons.at(i)->brainDelete(this);
    if (previous)
    {
        delete previous;
        previous = NULL;
    }
    inputNeurons.clear();
    outputNeurons.clear();
}

void BrainInterface::deleteLater(Neuron *neuron)
{
    if (previous)
        delete previous;
    previous = neuron;
}
