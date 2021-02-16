#include <iostream>
#include <deque>
#include <conio.h>
#include <chrono>
#include <windows.h>
#include "Event.h"

#include <timer.h>

using namespace std;

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

protected:
    /**
     *  Main condition: The condition that an event happened is if something is in the
     *  event queue. The same condition counts for the reset call because the elemnt must be
     *  removed after the call to the event-method.
     *  NOTE: If the element can't be removed, the event-handler won't work!!!
     *        First, the event-method will be called in an endless loop.
     *        Second, the queue will fill up and won't accept any new events, if there is a limit.
     */
    virtual bool main_condition(void)
    {
        return (this->event_queue.size() > 0);
    }

    /**
     *  Sub condition: The sub condition is always true as there is only one event-action and
     *  no need to filter out any actions of the event.
     *  The sub condition is needed for inheriting classes where each child-class reacts
     *  to different event-actions.
     * 
     *  Example: mouse input event
     *  Class "MouseActionEvent" reacts to all event-actions: left- and rightclick-action.
     *  Class "MouseLeftClickEvent", inherited from "MouseActionEvent", only reacts to leftclick-action.
     *  Class "MouseRightClickEvent", inherited from "MouseActionEvent", only reacts to rightclick-action.
     */
    virtual bool sub_condition(void)
    {
        return true;
    }

    /**
     *  The reset method is called after the event-function-call.
     *  This method defines what is happening after the event was called.
     *  Usually the data from the called event gets removed.
     */
    virtual void reset(void)
    {
        this->event_queue.pop_front();
    }

public:
    /**
     *  The push method iterates through every event-object and pushes the event-data into their queues.
     *  In this case a character, read from the keyboard, is pushed into the event-object's queues.
     *  This method must be STATIC as it iterates through every instance of this event.
     */
    static void push(char c)
    {
        uint64_t t0 = timer::ns();
        for(void* instance : get_instances())
        {
            KeyEvent* event = (KeyEvent*)instance;
            if(event->event_queue.size() < 4)
                event->event_queue.push_back(c);
        }
        std::cout << timer::ns() - t0 << "ns (push)" << std::endl;
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
    KeyEvent key_event1;

protected:
    // register the events with their matching event-methods
    void init(void)
    {
        this->register_event(key_event1, (Listener::EventFunc)on_keybd1);
        this->register_event(key_event1, (Listener::EventFunc)on_keybd2);
        this->set_interval(std::chrono::milliseconds(5));
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

// Global value for a simple implementation, not the recommended way to do!!!
uint64_t time0, time1;
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
        // Let the listener run at full speed.
        // This is not recommended as it will force your CPU run at full speed.
        this->set_interval(std::chrono::milliseconds(0));
    }

public:
    MyListener2(void)
    {
        this->init();
    }

    static void on_keybd1(KeyEvent& event)
    {
        time1 = timer::ns();
        uint64_t delay = time1 - time0;
        std::cout << delay / 1000 << "us delay in first method" << std::endl;
    }

    static void on_keybd2(KeyEvent& event)
    {
        time1 = timer::ns();
        uint64_t delay = time1 - time0;
        std::cout << delay / 1000 << "us delay in second method" << std::endl;
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

    while(!GetAsyncKeyState(VK_ESCAPE))
    {
        if(kbhit())
        {
            time0 = timer::ns();
            KeyEvent::push(getch());
        }
        Sleep(5);
    }

    event_handler.stop();
    event_handler.cleanup();
    return 0;
}
