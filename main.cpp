/** The example demonstrates the intended usage of this event handler. */

#include <iostream>
#include <deque>
#include <chrono>   
#include <conio.h>  // only works on windows

#include "event.h"

/**
 *  Example event.
 *  This event listens to key inputs.
 */
class KeyEvent : public Event<KeyEvent>
{
private:
    // Create an event-queue to be able to queue multiple event inputs,
    // needed for the case if the event listener can't keep up.
    std::deque<char> event_queue;
    mingw_stdthread::mutex m;

protected:
    /**
     *  The condition that an event happened is if something is in the
     *  event queue.
     */
    virtual bool trigger(void)
    {
        return (this->event_queue.size() > 0);
    }

    /**
     *  The reset method is called after the event-function-call.
     *  This method defines what is happening after the event was called.
     *  Usually the data from the called event gets removed.
     */
    virtual void reset(void)
    {
        // Synchronization made by the creator of this event
        this->m.lock();
        this->event_queue.pop_front();
        this->m.unlock();
    }

public:
    /**
     *  The push method iterates through every event-object and pushes the event-data into their queues.
     *  In this case a character, read from the keyboard, is pushed into the event-object's queue.
     *  This method must be STATIC as it iterates through every instance of this event.
     *  The internal method must be called that the event handler can work properly.
     */
    static void push(char c)
    {
        // Iterate through every event...
        for(void* instance : get_instances())
        {
            // cast void* to pointer of event class
            KeyEvent* event = static_cast<KeyEvent*>(instance);
            // queue a maximum of 4 events (only as example)
            if(event->event_queue.size() < 4)
            {
                // Synchronization made by the creator of this event
                event->m.lock();
                event->event_queue.push_back(c);
                event->m.unlock();
            }
            // a call to the internal method (must be done for every event instance)
            event->internal();
        }
    }

    /**
     *  Finally you can implement as many additional methods you want.
     *  In this case there is a method that retrieves the data from the event input.
     * 
     *  The method returns the character that is at the very begin of the queue.
     *  Event-queues should always be first in, first out.
     */
    char get_char(void) {return this->event_queue.at(0);}
};

/** 
 *  First example listener.
 *  In this listener a basic output is inplemented.
 */
class MyListener : public Listener
{
private:
    // declare as many events you want
    KeyEvent key_event1, key_event2;

protected:
    // register the events with their matching event-methods
    void init(void)
    {
        // cast method to Listener::EventFunc
        this->register_event(key_event1, reinterpret_cast<Listener::EventFunc>(on_keybd1));
        this->register_event(key_event2, reinterpret_cast<Listener::EventFunc>(on_keybd2));
    }

public:
    // the registration process must take place in the constructor
    MyListener(void)
    {
        this->init();
    }

    // create as many event methods you want, multiple per event-object are allowed
    static void on_keybd1(KeyEvent& event)
    {
        std::cout << "From Listener1 / Function 1: " << event.get_char() << std::endl;
    }

    static void on_keybd2(KeyEvent& event)
    {
        std::cout << "From Listener1 / Function 2: " << event.get_char() << std::endl;
    }
};

/** 
 *  Second example listener.
 *  In this listener a time delay measurement will be implemented.
 */
class MyListener2 : public Listener
{
private:
    // declare as many events you want
    KeyEvent key_event1;

protected:
    // register the events with their matching event-methods
    virtual void init(void)
    {
        // cast method to Listener::EventFunc
        this->register_event(key_event1, reinterpret_cast<Listener::EventFunc>(on_keybd1));
        this->register_event(key_event1, reinterpret_cast<Listener::EventFunc>(on_keybd2));
    }

public:
    // the registration process must take place in the constructor
    MyListener2(void)
    {
        this->init();
    }

    // create as many event methods you want, multiple per event-object are allowed
    static void on_keybd1(KeyEvent& event)
    {
        std::cout << "From Listener 2 / Function 1: " << event.get_char() << std::endl;
    }

    static void on_keybd2(KeyEvent& event)
    {
        std::cout << "From Listener 2 / Function 2: " << event.get_char() << std::endl;
    }
};

/*
*   Initialize the event handler:
*   Create the listener objects and add it to the handler.
*/
void init_event_handler(EventHandler& handler)
{
    /*  In this example dynamic listeners are used.
        You can also use static listeners. */
    Listener* my_listener = new MyListener();
    Listener* my_listener2 = new MyListener2();

    // add listeners to the event handler
    handler.add_listener(my_listener);
    handler.add_listener(my_listener2);
}

// its showtime
int main()
{
    EventHandler event_handler(ListenerType::DYNAMIC_LISTENER); // create event handler
    init_event_handler(event_handler);                          // initialize event handler
    event_handler.start();                                      // start event handler
    std::cout << "handler started" << std::endl;

    /** Initialize and/or do other stuff that uses the event handler. */

    char c = 0;
    while(c != 27)
    {
        c = getch();            // If caracter was received...
        if(c != 27)             // and its not ESC...
            KeyEvent::push(c);  // call the KeyEvent.
    }

    // stop and cleanup is also dont by the destructor
    event_handler.stop();
    event_handler.cleanup();

    std::cout << "handler stopped" << std::endl;
    return 0;   // you have been terminated
}