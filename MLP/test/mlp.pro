#-------------------------------------------------
#
# Project created by QtCreator 2015-05-16T21:26:03
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = mlp
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    NetNeurons/neuron.cpp \
    NetNeurons/perceptron.cpp

HEADERS += \
    NetNeurons/neuron.h \
    NetNeurons/perceptron.h
