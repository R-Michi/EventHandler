#include "../event.h"
#include <mutex>

Listener::Listener(void) noexcept
{
    this->running = false;      // thread is not running at constructor
}

Listener::~Listener(void)
{
    this->stop();               // stop listener when constructor is called
}

void Listener::start(void)  
{
    if(!this->running)          // only start listener if listener is not already running
    {
        this->running = true;   // set listener thread in running state
        this->listener_thread = std::thread(listen, this);  // start listener thread
    }
}

void Listener::stop(void)
{
    if(this->running)                   // only stop if listener is runing
    {
        this->running = false;          // shutdown listener thread
        this->cv.notify_one();          // unblock listener thread
        this->listener_thread.join();   // wait for listener thread to finish
    }
}

bool Listener::event_has_happened(EventBase** ret_event, std::vector<EventFunc>** ret_functions) noexcept
{
    for(auto iter = this->event2func.begin(); iter != this->event2func.end(); iter++)
    {
        if(iter->first->trigger())
        {
            *ret_event = iter->first;       // return triggered event
            *ret_functions = &iter->second; // return functions to be called
            return true;                    // return true if one event has been triggered
        }
    }

    // if no event has been triggered return nullptr and false respectively
    *ret_event = nullptr;       
    *ret_functions = nullptr;
    return false;
}

void Listener::register_event(EventBase& event, EventFunc func)
{
    this->event2func[&event].push_back(func);   // add function to current event
    event.set_cv(&this->cv);                    // set condition variable to event
}

void Listener::listen(Listener* l)
{
    std::mutex mtx;
    std::unique_lock<std::mutex> lk(mtx);

    while(l->running)   
    {
        // wait until event has happened or listener stopps
        std::vector<EventFunc>* event_functions = nullptr;  // pointer to the triggered event
        EventBase* event = nullptr;                         // pointer to the functions of the triggered event

        // wait until event has been triggered or listener should shutdown
        l->cv.wait(lk, 
        [l, &event, &event_functions]
        {
            return ((bool)!l->running) || l->event_has_happened(&event, &event_functions);
        });

        if(l->running)  // only execute event functions if listener is running
        {
            // execute event functions of triggered event
            for(size_t i=0; i < event_functions->size(); i++)
                event_functions->at(i)(*event);
                
            // reset current event
            event->reset();
        }
    }
    lk.unlock();    // manually unlock if it may be locked
}
