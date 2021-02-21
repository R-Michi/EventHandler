#include "../event.h"

EventHandler::EventHandler(void) noexcept : EventHandler(ListenerType::DYNAMIC_LISTENER) {}  // default initialization with dynamic listeners

EventHandler::EventHandler(ListenerType lt) noexcept
{
    this->running = false;                  // event handler is not running by default
    this->listener_type = lt;
}

EventHandler::~EventHandler(void)
{
    this->cleanup();                        // automatically cleanup in destructor
}

void EventHandler::start(void)
{
    if(!this->running)                      // if handler is already started, do nothing
    {
        this->running = true;               // set to running starte
        for(Listener* l :this->listeners)   // start all listeners
            l->start();
    }
}

void EventHandler::stop(void)
{
    if(this->running)                       // if handler is already stopped, do nothing       
    {
        this->running = false;              // shutdown event handler
        for(Listener* l : this->listeners)  // stop all listeners
            l->stop();
    }
}

void EventHandler::cleanup(void)
{
    this->stop();                           // event handler MUST NOT be cleaned up before stop
    
    // automatic delete of listener objects (free memory) if listeners are dynamic
    if(this->listener_type == ListenerType::DYNAMIC_LISTENER)
    {
        for(Listener*& l : listeners)       
        {
            delete(l);
            l = nullptr;
        }
    }
    this->listeners.clear();                // clear internal vector that contains all listeners
}

void EventHandler::add_listener(Listener* l)
{
    if(!this->running)                      // listeners MUST NOT be added if event handler is running -> method does nothing
        this->listeners.push_back(l);
}
