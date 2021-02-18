#include <iostream>
#include <deque>
#include <conio.h>
#include <chrono>
#include <windows.h>
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
     *  Main condition: The condition that an event happened is if something is in the
     *  event queue. The same condition counts for the reset call because the elemnt must be
     *  removed after the call to the event-method.
     *  NOTE: If the element can't be removed, the event-handler won't work!!!
     *        First, the event-method will be called in an endless loop.
     *        Second, the queue will fill up and won't accept any new events, if there is a limit.
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
        this->m.lock();
        this->event_queue.pop_front();
        this->m.unlock();
    }

public:
    /**
     *  The push method iterates through every event-object and pushes the event-data into their queues.
     *  In this case a character, read from the keyboard, is pushed into the event-object's queues.
     *  This method must be STATIC as it iterates through every instance of this event.
     *  The internal method must be called that the event handler can work properly.
     */
    static void push(char c)
    {
        for(void* instance : get_instances())
        {
            KeyEvent* event = (KeyEvent*)instance;
            if(event->event_queue.size() < 4)
            {
                event->m.lock();
                event->event_queue.push_back(c);
                event->m.unlock();
            }
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
        this->register_event(key_event1, (Listener::EventFunc)on_keybd1);
        this->register_event(key_event2, (Listener::EventFunc)on_keybd2);
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
    KeyEvent key_event1;

protected:
    virtual void init(void)
    {
        this->register_event(key_event1, (Listener::EventFunc)on_keybd1);
        this->register_event(key_event1, (Listener::EventFunc)on_keybd2);
    }

public:
    MyListener2(void)
    {
        this->init();
    }

    static void on_keybd1(KeyEvent& event)
    {
        std::cout << "From Listener 2 / Function 1: " << event.get_char() << std::endl;
    }

    static void on_keybd2(KeyEvent& event)
    {
        std::cout << "From Listener 2 / Function 2: " << event.get_char() << std::endl;
    }
};

void init_event_handler(EventHandler& handler)
{
    Listener* my_listener = new MyListener();
    Listener* my_listener2 = new MyListener2();
    handler.add_listener(my_listener);
    handler.add_listener(my_listener2);
}

int main()
{
    EventHandler event_handler;
    init_event_handler(event_handler);
    event_handler.start();
    std::cout << "handler started" << std::endl;

    char c = 0;
    while(c != 27)
    {
        c = getch();
        if(c != 27)
            KeyEvent::push(c);
    }

    event_handler.stop();
    event_handler.cleanup();

    std::cout << "handler stopped" << std::endl;
    return 0;
}