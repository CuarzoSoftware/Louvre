#include "Output.h"
#include <LPainter.h>
#include <unistd.h>
#include <stdio.h>

Output::Output():LOutput(){}

LRegion region;
LRegion region2;
UInt32 i = 0;

void Output::initializeGL()
{

}

void Output::paintGL()
{
    painter()->clearScreen();

    region.addRect(LRect(200));
    region.addRect(LRect(150, 150, 100, 100));
    region.subtractRect(LRect(210,210,10,10));

    region2.copy(region);
    region2.multiply(0.1);


    for(const LRect &r : region.rects())
    {
        float R = 0.3f + float(rand() % 1000)/2000.f;
        float G = 0.3f + float(rand() % 1000)/2000.f;
        float B = 0.5f + float(rand() % 1000)/2000.f;
        painter()->drawColor(r,R,G,B,1);
    }

    region.clear();

    /*
    glEnable(GL_BLEND);

    Int32 x = rand() % 1200;
    Int32 y = rand() % 1200;
    Int32 w = rand() % 1200 + 20;
    Int32 h = rand() % 1200 + 20;

    if(rand() % 10 < 8)
        region.addRect(LRect(x,y,w,h));
    else
        region.subtractRect(LRect(x,y,w,h));

    for(const LRect &r : region.rects())
    {
        float R = 0.3f + float(rand() % 1000)/2000.f;
        float G = 0.3f + float(rand() % 1000)/2000.f;
        float B = 0.5f + float(rand() % 1000)/2000.f;
        painter()->drawColor(r,1,0,0,0.5);
    }

    repaint();
    return;

    // Add entire screen
    if(i == 0)
    {
        region.addRect(rect());
    }
    else if(i == 1)
        region.subtractRect(LRect(rect().w()/2-100,rect().h()/2-100,200,200));
    else if(i == 2)
        region.subtractRect(LRect(rect().w()/2-200,rect().h()/2-200,200,200));
    else if(i == 3)
        region.subtractRect(LRect(rect().w()/2,rect().h()/2,200,200));
    else if(i == 4)
        region.subtractRect(LRect(rect().w()/2-300,rect().h()/2+100,200,200));
    else if(i == 5)
        region.subtractRect(LRect(rect().w()/2+100,rect().h()/2-300,200,200));
    else if(i == 6)
        region.subtractRect(LRect(0,0,100,100));
    else if(i == 7)
        region.subtractRect(LRect(rect().w()-100,rect().h()-100,100,100));
    else if(i == 8)
        region.subtractRect(LRect(0,rect().h()-100,100,100));
    else if(i == 9)
        region.subtractRect(LRect(rect().w()-100,0,100,100));
    else if(i == 10)
    {
        region.subtractRect(LRect(100,100,rect().w()-200,rect().h()-200));
    }
    else if(i < 200)
    {
        Int32 w = (rect().w()-200)/10;
        Int32 h = (rect().w()-200)/10;
        region.addRect(LRect((i-11)*10,(i-11)*10,w,h));
    }
    else if (i < 300)
    {
        Int32 x = i-200;
        region.subtractRect(LRect(x*50,x*50,25,25));
    }
    else
    {
        Int32 l = (i-300)*10;
        region.clip(LRect(l,l,rect().w()-l*2,rect().h()-l*2));
    }


    for(const LRect &r : region.rects())
    {
        float R = 0.3f + float(rand() % 1000)/2000.f;
        float G = 0.3f + float(rand() % 1000)/2000.f;
        float B = 0.5f + float(rand() % 1000)/2000.f;
        painter()->drawColor(r,R,G,B,1);
    }

    if(i<11)
        usleep(700000);
    else
        usleep(10000);
    i++;
    */
    //repaint();
}
