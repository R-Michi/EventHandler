#include "../Event.h"

EventHandler::EventHandler(void) : EventHandler(ListenerType::DYNAMIC_LISTENER) {}

EventHandler::EventHandler(ListenerType lt)
{
    this->running = false;
    this->listener_type = lt;
}

EventHandler::~EventHandler(void)
{
    this->cleanup();
}

void EventHandler::start(void)
{
    if(!this->running)
    {
        this->running = true;
        for(Listener* l :this->listeners)
            l->start();
    }
}

void EventHandler::stop(void)
{
    if(this->running)
    {
        this->running = false;
        for(Listener* l : this->listeners)
            l->stop();
    }
}

void EventHandler::cleanup(void)
{
    this->stop();
    // delete every listener
    if(this->listener_type == ListenerType::DYNAMIC_LISTENER)
    {
        for(Listener*& l : listeners)
        {
            delete(l);
            l = nullptr;
        }
    }
    this->listeners.clear();
}

void EventHandler::add_listener(Listener* l)
{
    if(!this->running)
        this->listeners.push_back(l);
}
