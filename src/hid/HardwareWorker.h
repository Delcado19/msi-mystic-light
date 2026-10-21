#pragma once
#include <QObject>
#include <QThread>
#include <utility>

// All runtime driver operations share one queue, preserving mode-before-frame
// ordering without allowing slow USB reports to block the GUI or overlap HID IO.
class HardwareWorker {
public:
    HardwareWorker() {
        context_=new QObject;context_->moveToThread(&thread_);
        QObject::connect(&thread_,&QThread::finished,context_,&QObject::deleteLater);
        thread_.setObjectName("Lighting hardware");thread_.start();
    }
    ~HardwareWorker(){stop();}
    template<class F> void post(F&& operation){QMetaObject::invokeMethod(context_,std::forward<F>(operation),Qt::QueuedConnection);}
    QObject* context()const{return context_;}
    void stop(){if(thread_.isRunning()){thread_.quit();thread_.wait();}}
private:
    QThread thread_;QObject* context_;
};
