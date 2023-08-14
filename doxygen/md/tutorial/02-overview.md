# Quick Overview

Let's start by taking a concise look at how Louvre works.

## Interface Methods

First and foremost, it's crucial to understand that Louvre employs virtual methods as interfaces to respond to signals or client application requests. Consider the example of the Louvre::LSurface class, which represents a client window:

```cpp

using namespace Louvre;

class YourCustomSurface : public LSurface
{
	...
    
    void bufferSizeChanged() override
    {
    	// Here your handle this signal
    }
    
    void mappingChanged() override
    {
    	// Here your handle this signal
    }
    
    ...
};
```

These methods are not intended for direct invocation by you; rather, they are triggered by the library itself to notify you of changes, allowing you to handle them effectively. 

You don't need to override all the methods, just the ones that are relevant to your use case, as the library provides default way for handling them. 

To see the default implementation provided by the library, please refer to the API documentation. For example, visit the following links: Louvre::LSurface::bufferSizeChanged() and Louvre::LSurface::mappingChanged().

## Main Classes

Louvre provides a diverse range of classes, each serving specific purposes. Here's a quick overview of some crucial classes that will guide you in navigating the documentation effectively:

### LCompositor

This is the heart of every Louvre compositor, responsible for tasks such as creating other classes, loading graphics and input backends, processing events, handling client requests, and much more. One particularly significant aspect to understand is that it includes multiple virtual constructors and destructors for various other classes.

For instance, when a client wishes to create a new surface, the Louvre::LCompositor::createSurfaceRequest() virtual method is invoked:

```cpp

using namespace Louvre;

class YourCustomCompositor : public LCompositor
{
	...
    
    LSurface *createSurfaceRequest(LSurface::Params *params) override
    {
    	return new YourCustomSurface(params);
    }
    
    void destroySurfaceRequest(LSurface *surface) override
    {
    	// Only notifies; you should not call 'delete surface;'
    }
    
    ...
};
```

As you might deduce, each virtual constructor/destructor has a default implementation provided by the library as well. For example, the default implementation of Louvre::LCompositor::createSurfaceRequest() returns a new LSurface instance, rather than an instance of the YourCustomSurface class.

Therefore, if you intend to use your own classes, you should override their respective virtual constructors.

It's essential to note that not all classes need to be created in this manner; this approach is specifically applicable to client, backend, and internal compositor resources.

### LOutput

The Louvre::LOutput class serves as a representation of a screen or display. Its primary responsibility is handling rendering for a specific screen. This class offers a range of essential virtual methods, including Louvre::LOutput::initializeGL(), Louvre::LOutput::paintGL(), Louvre::LOutput::resizeGL(), and others. You have the flexibility to override these methods to customize and perform your own painting operations.

### LPointer

Louvre::LPointer is a singleton class responsible for capturing input events from devices like a mouse or touchpad. It offers various virtual methods for event listening and enables forwarding these events to client applications.

### LKeyboard

Louvre::LKeyboard is similar to LPointer but for system keyboard events. It allows you to listen to keyboard events, forward them to clients, and configure keyboard aspects such as layout, key press repeat rate, and more.

In the upcoming chapters, we will explore the practical usage of these classes, and many more.