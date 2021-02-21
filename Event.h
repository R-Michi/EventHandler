/******************************************************************************************************************************************
* @title:        Event handler
* @author:       Michael Reim
* @date:         21.02.2021
* @description:
*   This library includes a general purpose event handler.
*   As events get handled asynchronously, synchronization must be made BY THE USER.
*   
*   If there are any bugs in this library please report it to the following link:
*   https://github.com/R-Michi/EventHandler/issues
*   Note that the creator of the library does not guarantee full functionality at improper use.
*   How to use the event handler the intended way, have a look at the documentation.
*
* @version release 1.3.1
* @copyright (C) Michael Reim, distribution without my consent is prohibited.
******************************************************************************************************************************************/

#ifndef __event_h__
#define __event_h__

#include <map>      // for std::map
#include <vector>   // for std::vector
#include <list>     // for std::list
#include <atomic>   // for atomic memory

/*  std::thread, std::mutex, std::condition_variable, etc. is not supported at the mingw compiler.
*   However, there is a library that implements these missing things with the exact same functionality.
*   Library can be found here: https://github.com/meganz/mingw-std-threads
*   If std::thread, etc. is aviable, use it instead the mingw implementaion.
*/
#if defined(_GLIBCXX_HAS_GTHREADS) && defined(_GLIBCXX_USE_C99_STDINT_TR1)
    #include <thread>
    #include <condition_variable>
#else
    #include <mingw.thread.h>
    #include <mingw.condition_variable.h>
#endif

/**
 *  enum: ListenerType
 *  Defines wether listenere objects are allocated statically or dynamically.
 */
enum ListenerType
{
    DYNAMIC_LISTENER,   // listener object is allocated as dynamic memory
    STATIC_LISTENER     // listener object is allocated as static memory
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
    *   This method is used as the trigger for the event-call.
    *   The event gehts called if the method returnes 'true'.
    *   Method must be implemented by the creator of an event.
    */
    virtual bool trigger(void) = 0;

    /**
    *   The reset-method defines what should happen after the event-call.
    *   Method must be implemented by the creator of an event.
    */
    virtual void reset(void) = 0;

public:
    EventBase(void)             noexcept    {this->cv = nullptr;}
    virtual ~EventBase(void)    noexcept    {}
};

/**
*   class: Event
*   EVERY EVENT MUST INHERIT FROM THIS CLASS, NOT FROM EventBase.
*
*   The template is used that every inheriting event has its own set of static members.
*   Example: "MyEvent : public Event<MyEvent>"
*/
template<typename T>
class Event : public EventBase
{
protected:
    using instance_iterator = std::list<void*>::iterator;

private:
    inline static std::list<void*> instances;   // contains every instance of a SINGLE event class.
    instance_iterator instance_iter;            // iterator to 'this' class

protected:
    /**
    *   Method to access all instances if 'this' event.
    *   As every event has its own set of static members, this method does not return
    *   instances of other events!
    *   NOTE: It returns a list of void pointers that only contain the addresses of the instances.
    *   A cast to the actual event-class MUST be made.
    */
    static const std::list<void*>& get_instances(void) noexcept {return instances;}

    /**
     *  This is a method that does some internal stuff.
     *  If you call the event and you iterate through every instance, every instance must call this 
     *  method at the very end. Otherwise your listener will not work!!!
     */
    void internal(void) noexcept
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
    virtual ~Event(void) noexcept
    {
        instances.erase(this->instance_iter);
    }
};

/**
*   class: Listener
*   The listener checks if an event has occured and calls the matching event function.
*   Every listener object operates asynchronous, in a different thread.
*   NOTE: Input and output is asynchronous, you might have to synchronize them in chertain conditions!
*   Starting and stopping listeners can only be done by the event handler.
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

    std::map<EventBase*, std::vector<EventFunc>> event2func;    // address of an event is unique
    std::atomic_bool running;

    static void listen(Listener*);  // function that is called within the thread
    void start(void);               // starts the listener thread
    void stop(void);                // stops the listener thread
    bool event_has_happened(EventBase**, std::vector<EventFunc>**) noexcept;

public:
    Listener(void) noexcept;

    // there is no need to copy of move listeners
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
    EventHandler(void) noexcept;
    explicit EventHandler(ListenerType) noexcept;

    // there is no need to copy or move event handlers
    EventHandler(const EventHandler&)               = delete;
    EventHandler& operator= (const EventHandler&)   = delete;

    EventHandler(EventHandler&&)                    = delete;
    EventHandler& operator= (EventHandler&&)        = delete;

    virtual ~EventHandler(void);

    // Starts every listener thread.
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
    bool is_running(void) const noexcept {return this->running;}
};

#endif // __event_h__
