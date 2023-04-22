// logger.h
#pragma once
#include "cmp-timer.h"
#include "cmp-log.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <mutex>
#include <flecs.h>

namespace axiom {

    class Sys_Logger {
    public:
        Sys_Logger(flecs::world& world_) {
            world = &world_;
            auto e = world->singleton<Cmp_LogFile>();
            world->system<Cmp_LogFile>("StartLogFileSystem")
                .kind(flecs::OnStart)
                .each(&Sys_Logger::OpenLogFile);

            
            world->system<Cmp_LogFile>("ShutdownLogFileSystem")
                .kind(flecs::OnDelete)
                .each(&Sys_Logger::CloseLogFile);


            world->system<Cmp_Log>("LogSystem")
                .kind(flecs::OnStart)
                .each(&Sys_Logger::OnLog);
        }

        ~Sys_Logger() {
        }

        //This Creates/Opens the logfile
        void OpenLogFile(flecs::entity e, Cmp_LogFile& cmp){
            cmp.log_file = std::ofstream(cmp.log_filename, std::ios::app);
            if(!cmp.log_file.is_open()){
                std::cerr << "Failed to open log file: " << cmp.log_filename << std::endl;
            }
            std::cout << "Logfile started";
        }

        //This Closes the logfile
        void CloseLogFile(flecs::entity e, Cmp_LogFile& f){
            if(f.log_file.is_open()){
                f.log_file.close();
            }
            std::cout << "Log file removed";
        }

        //This Logs the message
        void OnLog(flecs::entity e, Cmp_Log& l){
            if(l.lvl == LogLevel::CHECK)
                CheckMessage(l.check, l.message);
            else
                LogMessage(l.lvl, l.message);
        }

        //Tihs logs the message
        void LogMessage(axiom::LogLevel level, std::string message){
            std::stringstream ss;
            auto* timer = world->get<Cmp_Timer>();
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
            }
            ss << message << std::endl;
            std::string log_line = ss.str();
            std::cout << log_line;
            SaveLog(log_line);
        }

        //this checks the mssage
        void CheckMessage(bool b, std::string message){
            if(b) LogMessage(LogLevel::ERROR ,"Failed When " + message);
            else LogMessage(LogLevel::INFO , message + " was a success!");
        }

        //This writes the log to a file
        void SaveLog(std::string message)
        {
            auto* file = world->get_mut<Cmp_LogFile>();
            std::unique_lock<std::mutex> lock(file->log_mutex);
            if (file->log_file.is_open()) {
                file->log_file << message;
                file->log_file.flush();
            }
            lock.unlock();
        }

    private:
        flecs::world* world;
    };

}

/*
Okay so have a log file system
on start it opens the file
on destruction it closes the file

Then have a log system
for
 - regular logs
 - a check system
 So like for getglfwwiondow() it'd be
 axiom::Log(Logger::Check, getglfwwindow(), "Opening GLFW WINdow") or something similar

/*
inline void LogDebug(const std::string& message) {
  Logger::GetInstance().Log(Logger::Level::DEBUG, message);
}

inline void LogInfo(const std::string& message) {
  Logger::GetInstance().Log(Logger::Level::INFO, message);
}

inline void LogWarning(const std::string& message) {
  Logger::GetInstance().Log(Logger::Level::WARNING, message);
}

inline void LogError(const std::string& message) {
  Logger::GetInstance().Log(Logger::Level::ERROR, message);
}

inline void Check(int condition, const std::string& message){
  Logger::GetInstance().ErrorCheck(condition, message);
}*/
