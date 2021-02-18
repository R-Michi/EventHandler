/******************************************************************************************************************************************
* Title:        Event handler
* Author:       Michael Reim
* Date:         17.02.2021
* Description:
*   General purpose library to create and listen to events.
*
* @version release 1.3.0
* @copyright (C) Michael Reim, distribution without my consent is prohibited.
*
* If there are any bugs, contact me!
******************************************************************************************************************************************/

#ifndef __event_h__
#define __event_h__

#include <map>
#include <vector>
#include <list>
#include <chrono>
#include <atomic>
#include <cstdlib>

// use std if aviable
#if defined(_GLIBCXX_HAS_GTHREADS) && defined(_GLIBCXX_USE_C99_STDINT_TR1)
    #include <thread>       // use std::thread if it is aviable
    #include <condition_variable>
#else
    #include <mingw.thread.h>
    #include <mingw.condition_variable.h>
#endif

/**
*   Type of the listener.
*   Defines wether a listener object is allocated dynamically or static.
*   The listener object will be allocated BY THE USER.
*/
enum ListenerType
{
    DYNAMIC_LISTENER,   // listener object is allocated as dynamic memory (object is in heap)
    STATIC_LISTENER     // listener object is allocated as static memory (object is in stack)
};

/**
*   class: EventBase
*   Is the abstract base class of every event.
*   This class contains some methods that every event has to implement.
*/
class EventBase
{   
    friend class Listener;

protected:
#if defined(_GLIBCXX_HAS_GTHREADS) && defined(_GLIBCXX_USE_C99_STDINT_TR1)
    std::condition_variable* cv;
    void set_cv(std::condition_variable* _cv) noexcept              {this->cv = _cv;}
#else
    mingw_stdthread::condition_variable* cv;
    void set_cv(mingw_stdthread::condition_variable* _cv) noexcept  {this->cv = _cv;}
#endif

    /**
    *   This method is used as the trigger for the event- and reset-call.
    *   Method must be implemented by the creator of an event.
    */
    virtual bool trigger(void) = 0;

    /**
    *   The reset-method defines what should happen if the event gets reset.
    *   Method must be implemented by the creator of an event.
    */
    virtual void reset(void) = 0;

public:
    EventBase(void)             {this->cv = nullptr;} 
    virtual ~EventBase(void)    {}
};

/**
*   class: Event
*   EVERY EVENT MUST INHERIT FROM THIS CLASS, NOT FROM EventBase.
*
*   The template is used that every inheriting event has its own set of static members.
*   Example: "MyEvent : public Event<MyEvent>"
*
*   This class contains useful functions to be able to generate events.
*/
template<typename T>
class Event : public EventBase
{
protected:
    using instance_iterator = std::list<void*>::iterator;

private:
    inline static std::list<void*> instances;   // contains every instance of a SINGLE event class.
    instance_iterator instance_iter;

protected:
    /**
    *   Method to access every single event instance.
    *   @return a list of all instances.
    *   NOTE: You have only access to all instances of ONE SINGLE event class.
    */
    static const std::list<void*>& get_instances(void)  {return instances;}

    /**
     *  This is a method that does some internal working such as notifying condition
     *  variables.
     *  This method MUST be called at the end of your push function (or function where you call the event). Otherwise
     *  your listener will block forever!
     */
    void internal(void) 
    {   
        if(this->cv != nullptr)
            this->cv->notify_one();
    }
    
public:
    Event(void)
    {
        instances.push_back(this);
        this->instance_iter = instances.end();
        --this->instance_iter;
    }
    virtual ~Event(void)
    {
        instances.erase(this->instance_iter);
    }
};

/**
*   class: Listener
*   The listener checks if an event occured and calls the matching event function.
*   Every listener object operates asynchronous, in a different thread.
*   NOTE: Input and output is asynchronous, you might have to synchronize them in chertain conditions!!!
*
*   To be able to listen to events you have to:
*   1) inherit from this class
*   2) create a constructor
*   3) declare (non static) event-objects
*   4) code STATIC methods for the declared event-bjects (multiple methods per event-object are allowed)
*   5) register the event-objects with their matching event-methods via register_event() in the CONSTRUCTOR.
*/
class Listener
{
    friend class EventHandler;

protected:
    typedef void(*EventFunc)(const EventBase&);

    /**
    *   Method to register an event-object with a matching event-method.
    *   Method can be used within inheriting classes for initialization purposes.
    *   @param event -> Event-object to register.
    *   @param func -> Event-method to register (function pointer).
    */
    void register_event(EventBase& event, EventFunc func);

private:
#if defined(_GLIBCXX_HAS_GTHREADS) && defined(_GLIBCXX_USE_C99_STDINT_TR1)
    std::thread listener_thread;
    std::condition_variable cv;
#else
    mingw_stdthread::thread listener_thread;
    mingw_stdthread::condition_variable cv;
#endif

    std::map<EventBase*, std::vector<EventFunc>> event2func;
    std::atomic_bool running;

    static void listen(Listener*);  // function that is called within the thread
    void start(void);               // starts the listener thread
    void stop(void);                // stops the listener thread
    bool event_has_happened(EventBase**, std::vector<EventFunc>**) noexcept;

public:
    Listener(void);

    Listener(const Listener&)               = delete;
    Listener& operator= (const Listener&)   = delete;

    Listener(Listener&&)                    = delete;
    Listener& operator= (Listener&&)        = delete;

    virtual ~Listener(void);
};

/**
*   class: EventHandler
*   This class contains and manages multiple listener objects.
*   The EventHandler is able to start and stop listener objects with a single call
*   to start() or stop() respectively.
*   Events will only be handled if the EventHandler is in the running state.
*   NOTE: You must use an EventHandler-object because it is the only way you can start
*   the listener threads.
*/
class EventHandler
{
private:
    std::vector<Listener*> listeners;
    ListenerType listener_type;
    bool running;

public:
    EventHandler(void);
    EventHandler(ListenerType);

    EventHandler(const EventHandler&)               = delete;
    EventHandler& operator= (const EventHandler&)   = delete;

    EventHandler(EventHandler&&)                    = delete;
    EventHandler& operator= (EventHandler&&)        = delete;

    virtual ~EventHandler(void);

    // Starts eventy listener thread.
    void start(void);

    // Stopps every listener thread.
    void stop(void);

    // Will remove every added listener-object from the EvenetHandler.
    void cleanup(void);

    // Adds a listener object to the EventHandler.
    void add_listener(Listener*);

    /**
    *   Checks if the EventHandler is running or stopped.
    *   @return 'true' if EventHandler is running and 'false' if EventHandler is not running.
    */ 
    bool is_running(void) {return this->running;}
};

#endif // __event_h__
