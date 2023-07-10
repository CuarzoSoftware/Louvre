#ifndef OUTPUT_H
#define OUTPUT_H

#include <LOutput.h>
#include <LSolidColorView.h>

using namespace Louvre;

class Compositor;

class Output : public LOutput
{
public:
    Output();
    Compositor *compositor() const;
    void initializeGL() override;
    void resizeGL() override;
    void paintGL() override;
    void uninitializeGL() override;

    LSolidColorView topBar = LSolidColorView(1.f, 1.f, 1.f, 0.5);
};

#endif // OUTPUT_H
