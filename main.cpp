#include <iostream>
//#include <thread>
//#include <chrono>
#include <random>

#include <windows.h>

#include "conQueue.h"

// Stopper class
class Stopper
{
public:

    Stopper(): stopFlag(false){}
    bool isStop(){return stopFlag;}
    bool stop(){stopFlag = true;}
private:
    bool stopFlag;
};

// Stopper init
Stopper stop;

// mutex
//std::mutex mut;
CONST HANDLE hMut = CreateMutex(NULL, FALSE, NULL);


// Request class
class Request
{
public:
    int rTime;
    int pTime;
};


// GetRequest. If NULL then stop!
// else get request
Request* GetRequest(Stopper stopSignal)
{
    // new Request
    Request* request = new Request();

    // get random time
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<int> distribution(0,10);

    int rTime = distribution(generator);

    request->rTime = rTime;
    //std::chrono::seconds sec(rTime);

    // sleep random time
    //std::this_thread::sleep_for(sec);
    Sleep((DWORD)rTime * 1000);

    // out info to std::cout
    //mut.lock();
    WaitForSingleObject(hMut, INFINITE);
    std::cout << "GetRequest\t" <<  request << "\t" << request->rTime << std::endl;
    ReleaseMutex(hMut);
    //mut.unlock();

    return request;
};


// ProcessRequest. If stop == stop then abort
// else process
void ProcessRequest(Request* request, Stopper stopSignal)
{
    // if stopSignal is stop the abort
    if(stopSignal.isStop())
    {
        return;
    }

    // get random time
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<int> distribution(0,10);

    int pTime = distribution(generator);

    request->pTime = pTime;
    //std::chrono::seconds sec(pTime);

    // sleep random time
    //std::this_thread::sleep_for(sec);
    Sleep((DWORD)pTime * 1000);

    // out info to std::cout
    //mut.lock();
    WaitForSingleObject(hMut, INFINITE);
    std::cout << "Process\t" << request << "\t" << request->rTime << "\t" << request->pTime << std::endl;
    ReleaseMutex(hMut);
    //mut.unlock();
}


// DeleteRequest
void DeleteRequest(Request* request)
{
    // if request != NULL
    if(request != nullptr)
    {
        // out info to std::cout
        //mut.lock();
        WaitForSingleObject(hMut, INFINITE);
        std::cout << "Delete\t" << request << "\t" << request->rTime << "\t" << request->pTime << std::endl;
        ReleaseMutex(hMut);
        //mut.unlock();

        // delete request
        delete request;
        request = nullptr;
    }
}


// queue init
conQueue<Request*> queueRequests;


// read all reuquest while stopping
//void readRequests()
DWORD WINAPI readRequests(CONST LPVOID lpParam)
{
    while(!stop.isStop())
    {
        Request *req = GetRequest(stop);
        if(req == nullptr)
        {
            stop.stop();
            break;
        }
        else
        {
            queueRequests.push(req);
        }
    }
}


// process all reuquest while stopping
//void processRequests()
DWORD WINAPI processRequests(CONST LPVOID lpParam)
{
    while(!stop.isStop())
    {
        Request *req = queueRequests.pop();
        ProcessRequest(req, stop);
        if(stop.isStop())
        {
            DeleteRequest(req);
            break;
        }
        DeleteRequest(req);
    }
}


// delete all request in queue
void deleteAllRequests()
{
    while(queueRequests.size() > 0)
    {
        Request *req = queueRequests.pop();
        DeleteRequest(req);
    }
}


// main thread
int main()
{
    // start and end time
    //std::chrono::time_point<std::chrono::system_clock> start, end;

    // init count of Threads
    int threadsReaderCount = 5;
    int threadsProcessCount = 5;

    if(nullptr == hMut)
    {
        std::cout << "Failed to create mutex" << std::endl;
    }

    // init requestThreadsReader
    //std::vector<std::thread> requestThreadsReader;
    HANDLE requestThreadsReader[threadsReaderCount];
    for(int i = 0; i < threadsReaderCount; ++i)
    {
        //requestThreadsReader.emplace_back(&readRequests);
        requestThreadsReader[i] = CreateThread(nullptr,0,&readRequests,hMut,0,nullptr);
        if(nullptr == requestThreadsReader[i])
        {
            std::cout << "Failed to create requestThreadsReader" << std::endl;
        }
    }

    // init start clock
    //start = std::chrono::system_clock::now();

    // init requestThreadsProcess
    //std::vector<std::thread> requestThreadsProcess;
    HANDLE requestThreadsProcess[threadsProcessCount];
    for(int i = 0; i < threadsProcessCount; ++i)
    {
        //requestThreadsReader.emplace_back(&processRequests);
        requestThreadsProcess[i] = CreateThread(nullptr,0,&processRequests,hMut,0,nullptr);
        if(nullptr == requestThreadsProcess[i])
        {
            std::cout << "Failed to create requestThreadsProcess" << std::endl;
        }
    }

    // work 30 seconds
    /*
    end = std::chrono::system_clock::now();
    int elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    while(elapsed_seconds < 30)
    {
        end = std::chrono::system_clock::now();
        elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    }
    */
    Sleep(30000);

    // stop signal
    stop.stop();

    // wait requestThreadsReader
    WaitForMultipleObjects((DWORD)threadsReaderCount, requestThreadsReader, TRUE, INFINITE);
    /*
    for(auto& thread: requestThreadsReader)
    {
        thread.join();
    }
*/
    // wait requestThreadsProcess
    WaitForMultipleObjects((DWORD)threadsProcessCount, requestThreadsProcess, TRUE, INFINITE);
    /*
    for(auto& thread: requestThreadsProcess)
    {
        thread.join();
    }
     */

    deleteAllRequests();

    // close threads
    for(int i = 0; i < threadsReaderCount; i++)
    {
        CloseHandle(requestThreadsReader[i]);
    }

    for(int i = 0; i < threadsProcessCount; i++)
    {
        CloseHandle(requestThreadsProcess[i]);
    }

    CloseHandle(hMut);

    return 0;
}