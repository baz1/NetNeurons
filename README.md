# NetNeurons

This project includes several implementations of artificial neural networks.
This library being very lightweight, the source files should be directly included
in any project that uses them. The Makefile only exists to generate the documentation.

## ESN - Echo State Network

### Presentation

Echo State Networks are a type of recurrent neural network.
This library enables a basic usage of this type of networks.

However, it is **still at a development stage**.

## MLP - MultiLayered Perceptron

### Presentation

This part of the library contains two interfaces for creating multilayered perceptrons:

1. The Perceptron class, which is designed to be efficient and may be used with multiple threads.

2. The Neuron class and its BrainInterface, less efficient but more flexible.
Its main advantage is that you can connect neurons as you wish inside the brain -
as long as you do not create a loop. This functionality is an improvement that
many major libraries do not propose.

### Warning

Even though it does work, this implementation uses only Super-SAB as a learning algorithm,
and does not prevent being stuck in a potential well.

It does therefore **not give good results for the learning phase**.
You should use another implementation such as the FANN (Fast Artificial Neural Network) library,
which is well-known, documented, simple, and will have a more accurate learning,
even though you will probably still encounter those potential well problems.
