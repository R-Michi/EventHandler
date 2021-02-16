/******************************************************************************************************************************************
* Title:        Event handler
* Author:       Michael Reim
* Date:         20.01.2021
* Description:
*   General purpose library to create and listen to events.
*
* @version release 1.2.0
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

#if defined(_GLIBCXX_HAS_GTHREADS) && defined(_GLIBCXX_USE_C99_STDINT_TR1)
    #include <thread>       // use std::thread if it is aviable
    using std::thread;
#else
    #include <mingw.thread.h>
    using mingw_stdthread::thread;
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
    /**
    *   This method is used as the condition for the event- and reset-call.
    *   Method must be implemented by the creator of an event.
    */
    virtual bool main_condition(void) = 0;

    /**
    *   This method is only used as additional condition for the event-call.
    *   The purpose of this method is to filter out some actions of events.
    *   Method must be implemented by the creator of an event.
    * 
    *   NOTE:   reset_condition = main_condition
    *           event_condition = main_condition & sub_condition
    *   
    *   If the event-call condition should be the same as the reset-call condition, return 'true'.
    */
    virtual bool sub_condition(void) = 0;

    /**
    *   The reset-method defines what should happen if the event gets reset.
    *   Method must be implemented by the creator of an event.
    */
    virtual void reset(void) = 0;

public:
    EventBase(void) {}          // constructor
    virtual ~EventBase(void) {} // destructor
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
    *   This method sets the interval in which the events are queried.
    *   Method can be used within inheriting classes for initialization purposes.
    *   @param interval -> Interval in which events are queried.
    */
    void set_interval(std::chrono::nanoseconds interval)    {this->listener_interval = interval;}

    /**
    *   Method to register an event-object with a matching event-method.
    *   Method can be used within inheriting classes for initialization purposes.
    *   @param event -> Event-object to register.
    *   @param func -> Event-method to register (function pointer).
    */
    void register_event(EventBase& event, EventFunc func)   {this->event2func[&event].push_back(func);}

private:
    std::map<EventBase*, std::vector<EventFunc>> event2func;
    thread listener_thread;
    std::atomic_bool running, thread_working;
    std::chrono::nanoseconds listener_interval;

    void start(void);   // starts the listener thread
    void stop(void);    // stops the listener thread
    static void listen(Listener*);  // function that is called within the thread

public:
    Listener(void); // listener runs with full speed as standard

    Listener(const Listener&) = delete;
    Listener& operator= (const Listener&) = delete;

    Listener(Listener&&) = delete;
    Listener& operator= (Listener&&) = delete;

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

    EventHandler(const EventHandler&) = delete;
    EventHandler& operator= (const EventHandler&) = delete;

    EventHandler(EventHandler&&) = delete;
    EventHandler& operator= (EventHandler&&) = delete;

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
