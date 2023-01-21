#ifndef OUTPUT_H
#define OUTPUT_H

#include <LOutput.h>

using namespace Louvre;

class Output : public LOutput
{
public:
    Output();
    void initializeGL() override;
    void paintGL() override;
    LTexture *bg;
};

#endif // OUTPUT_H
