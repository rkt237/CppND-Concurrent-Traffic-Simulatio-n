#include "TrafficLight.h"

#include <iostream>
#include <random>
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.

    // perform queue modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

    // remove last vector element from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg; // will not be copied due to return value optimization (RVO) in C++
}


template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // add vector to queue
    std::cout << "   Message " << msg << " has been sent to the queue" << std::endl;
    _queue.push_back(std::move(msg));
    _cond.notify_one(); // notify client after pushing new Vehicle into vector
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    setCurrentPhase( TrafficLightPhase::red );
}

TrafficLight::~TrafficLight()
{
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while (1)
    {
        if ( _msgQueue.receive() == TrafficLightPhase::green )
        {
            break;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::setCurrentPhase( const TrafficLightPhase CurrentPhase )
{
    _currentPhase = CurrentPhase;
}


void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back( std::thread( &TrafficLight::cycleThroughPhases, this ) );
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    const int low  = 4;
    const int high = 6;

    // start and end time
    std::chrono::time_point<std::chrono::system_clock> startTime, endTime;

    // declare variable to hold seconds on clock
    time_t seconds;

    // Get value from system clock and place in seconds variable
    time(&seconds);

    // convert seconds to a unsigned integer
    srand( (unsigned int) seconds );

    // random number
    int cycle_duration = rand() % (high - low + 1) + low;
    
    startTime = std::chrono::system_clock::now();
    
    while (1)
    {
        std::this_thread::sleep_for( std::chrono::milliseconds(1) );

        endTime = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed_seconds = endTime-startTime;

        if ( elapsed_seconds.count() > cycle_duration )
        {
            if ( _currentPhase == red )
            {
                _currentPhase = green;
            }
            else
            {
                _currentPhase = red;
            }

            cycle_duration = rand() % (high - low + 1) + low;
            
            startTime = std::chrono::system_clock::now();

            // sends an update method to the message queue
            _msgQueue.send(std::move(_currentPhase));
        }
    }

}

