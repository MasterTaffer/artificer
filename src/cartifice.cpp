
#include "cartifice.h"
#include "artifice.hpp"

//We use a separate thread for every artificer instance
//Because GLFW is thread local, we need a new thread
//Otherwise we might break something in the calling main thread
//Of course, this isn't a problem in application that do not use GLFW/OpenGL

#include <thread>
#include <mutex>
#include <future>

enum TM_ThreadMessage
{
    TM_INIT = 0x3000,
    TM_DEINIT,
    TM_DOFRAME,
    TM_DONE
};

enum TM_ThreadSender
{
    TM_MAIN = 0x9000,
    TM_SUB
};

int artificer_color_format_to_gl(int colformat)
{
    if (colformat == artificer_color_format_bgra)
        return GL_BGRA;
    return GL_RGBA;
}


//Struct for transmitting parameters and messages
struct TM_ThreadContainer
{
    artificer_parameters* params;

    unsigned char *to, *from;
    int framenum;

    Artificer* art;
    std::mutex mtx;
    int msg = 0;
    int hasMsg = 0;
    std::future<void> future;
};

/*
    Both threads follow the simple rule when messaging
    send -> wait for reply -> send another -> ...

    TM_Message() Send a message
    TM_ReceiveMessage() Wait for a message

*/

//Wait for a message from a specific sender
int TM_ReceiveMessage(TM_ThreadContainer* tc, int from)
{
    int gotmsg = 0;
    while (1)
    {
        {
            //Lock the data before checking for messages
            std::lock_guard<std::mutex> guard(tc->mtx);
            if (tc->hasMsg == from)
            {
                gotmsg = tc->msg;
                tc->hasMsg = 0;
                tc->msg = 0;
                break;
            }
        }
        //Hopefully the OS scheduler actually takes the hint
        std::this_thread::yield();
    }
    return gotmsg;
}

//Send a message, without locks
void TM_LocklessMessage(TM_ThreadContainer* tc, int msg, int sender)
{
    tc->msg = msg;
    tc->hasMsg = sender;
}

//Send a message
void TM_Message(TM_ThreadContainer* tc, int msg, int sender)
{
    std::lock_guard<std::mutex> guard(tc->mtx);
    TM_LocklessMessage(tc, msg,sender);
}

//This is the function the worker/slave thread spends its time

void TM_Loop(TM_ThreadContainer* tc)
{
    //First lock the data
    tc->mtx.lock();

    //Start and initialize the artificer
    tc->art = new Artificer();
    tc->art->init(tc->params->config_path, tc->params->width, tc->params->height,
                  tc->params->display_output, artificer_color_format_to_gl(tc->params->color_format));

    while (1)
    {
        //We're done with initilalizing / previous frame
        //Send a message telling that
        TM_LocklessMessage(tc,TM_DONE,TM_SUB);
        tc->mtx.unlock();

        //Wait for orders from main thread...
        int msg = TM_ReceiveMessage(tc,TM_MAIN);
        if (msg == TM_DEINIT)
            break;

        //If we need to render a frame:
        //lock the data and render the frame

        //data will be unlocked back at the top of the loop
        tc->mtx.lock();
        tc->art->processFrame(tc->from,tc->to,tc->framenum);
    }

    //Deinitializing... lock the data

    tc->mtx.lock();

    tc->art->deInit();
    delete tc->art;
    TM_LocklessMessage(tc,TM_DONE,TM_SUB);

    tc->mtx.unlock();

    //And done, now call tc->future.get()
}

void TM_DoCall(TM_ThreadContainer* tc, TM_ThreadMessage tm)
{
    //Send an order
    TM_Message(tc,tm,TM_MAIN);

    //Wait for the slave thread to do its thing
    //draw a new frame, deinitialize...

    TM_ReceiveMessage(tc,TM_SUB);
}

void TM_Init(TM_ThreadContainer* tc)
{
    tc->future = std::async(std::launch::async,TM_Loop,tc);
    TM_ReceiveMessage(tc,TM_SUB);
}

void TM_Done(TM_ThreadContainer* tc)
{
    tc->future.get();
}

//For low level "simple" api

extern "C"
{
    void* artificer_start(artificer_parameters* param)
    {
        param->error = 0;
        param->error_string = nullptr;

        auto p = new TM_ThreadContainer();

        p->params = param;


        //Create a slave thread & initialize artificer
        TM_Init(p);


        if (p->art->error)
        {

            param->error_string = p->art->errorb.c_str();
            param->error = p->art->error;

        }

        param->output_width = p->art->width;
        param->output_height = p->art->height;

        return p;
    }

    void artificer_process(void* art, unsigned char* from,unsigned char* to, int framenum)
    {
        TM_ThreadContainer* p = (TM_ThreadContainer*)art;
        p->framenum = framenum;
        p->from = from;
        p->to = to;

        TM_DoCall(p,TM_DOFRAME);
    }

    void artificer_end(void* art)
    {
        TM_ThreadContainer* p = (TM_ThreadContainer*)art;

        TM_DoCall(p,TM_DEINIT);
        TM_Done(p);

        delete (p);
    }


    artificer_parameters artificer_default_parameters()
    {
        artificer_parameters pm;

        pm.color_format = artificer_color_format_rgba;
        pm.display_output = 0;
        pm.width = 0;
        pm.height = 0;
        pm.config_path = nullptr;
        pm.output_height = 0;
        pm.output_width = 0;

        return pm;
    }

}


