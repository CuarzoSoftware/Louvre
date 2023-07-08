#ifndef OUTPUT_H
#define OUTPUT_H

#include <LOutput.h>

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
};

#endif // OUTPUT_H
