#include "pch.h"
#include "sys-log.h"
#include "sys-timer.h"
namespace axiom{
    Sys_Logger::Sys_Logger(flecs::world& world_) {
        world = &world_;
        world->observer<Cmp_LogFile>("StartLogFileSystem")
            .event(flecs::OnAdd)
            .each([this](flecs::entity e, Cmp_LogFile& f){
                this->OpenLogFile(e,f);
        });

        world->observer<Cmp_LogFile>("ShutdownLogFileSystem")
            .event(flecs::OnRemove)
            .each([this](flecs::entity e, Cmp_LogFile& f){
                this->CloseLogFile(e,f);
        });

        world->observer<Cmp_Log>("OnSetLog")
            .event(flecs::OnSet)
            .each([this](flecs::entity e, Cmp_Log& f){
                this->OnLog(e,f);
        });
    }

    Sys_Logger::~Sys_Logger(){}

    void Sys_Logger::OpenLogFile(flecs::entity e, Cmp_LogFile& cmp) 
    {
        cmp.log_data->log_file.open(cmp.log_filename, std::ios::app);
        if (!cmp.log_data->log_file.is_open()) {
            std::cerr << "Failed to open log file: " << cmp.log_filename << std::endl;
        }
        LogMessage(LogLevel::INFO, "Log File Opened");
    }

    void Sys_Logger::CloseLogFile(flecs::entity e, Cmp_LogFile& f) {
        if (f.log_data->log_file.is_open()) {
            f.log_data->log_file << current_time() << " - [INFO] File Closed\n";
            f.log_data->log_file.close();
        }
        current_time();
        //
    }

    void Sys_Logger::SaveLog(std::string message) {
        auto* file = world->get_mut<Cmp_LogFile>();
        std::unique_lock<std::mutex> lock(file->log_data->log_mutex);
        if (file->log_data->log_file.is_open()) {
            file->log_data->log_file << message;
            file->log_data->log_file.flush();
        }
        lock.unlock();
    }

    void Sys_Logger::OnLog(flecs::entity e, Cmp_Log& l){
        if(l.lvl == LogLevel::CHECK)
            CheckMessage(l.check, l.message);
        else
            LogMessage(l.lvl, l.message);
    }

    void Sys_Logger::LogMessage(axiom::LogLevel level, std::string message){
        std::stringstream ss; 
        ss << current_time() << " - ";
        switch (level) {
            case LogLevel::DEBUG:
                ss << "[DEBUG] ";
                break;
            case LogLevel::INFO:
                ss << "[INFO] ";
                break;
            case LogLevel::WARNING:
                ss << "[WARNING] ";
                break;
            case LogLevel::ERROR:
                ss << "[ERROR] ";
                break;
            case LogLevel::CHECK:
                break;
        }
        ss << message << std::endl;
        std::string log_line = ss.str();
        std::cout << log_line;
        SaveLog(log_line);
    }
    
    void Sys_Logger::CheckMessage(bool b, std::string message){
        if(b) LogMessage(LogLevel::ERROR ,"Failed When " + message);
        else LogMessage(LogLevel::INFO , message + " was a success!");
    }
}