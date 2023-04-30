#include "pch.h"
#include "sys-log.h"
#include "sys-timer.h"
namespace axiom{
    namespace log{
        void Init() 
        {
            // Open/Start Log File System
            g_world.observer<Cmp_LogFile>("StartLogFileSystem")
                .event(flecs::OnAdd)
                .each([](flecs::entity e, Cmp_LogFile& f){
                    f.log_data->log_file.open(f.log_filename, std::ios::app);
                    if (!f.log_data->log_file.is_open()) {
                        std::cerr << "Failed to open log file: " << f.log_filename << std::endl;
                    }
                    Set(LogLevel::INFO, "Log File Opened");
            });

            // CLose file
            g_world.observer<Cmp_LogFile>("ShutdownLogFileSystem")
                .event(flecs::OnRemove)
                .each([](flecs::entity e, Cmp_LogFile& f){
                    if (f.log_data->log_file.is_open()){
                        f.log_data->log_file << timer::Current() << " - [INFO] File Closed\n";
                        f.log_data->log_file.close();
                    }
                timer::Current();
            });

            // Log File
            g_world.observer<Cmp_Log>("OnSetLog")
                .event(flecs::OnSet)
                .each([](flecs::entity e, Cmp_Log& l){
                    if(l.lvl == LogLevel::CHECK)
                        Check(l.check, l.message);
                    else
                        Set(l.lvl, l.message);
            });
        }

        void Save(std::string message) {
            auto* file = g_world.get_mut<Cmp_LogFile>();
            std::unique_lock<std::mutex> lock(file->log_data->log_mutex);
            if (file->log_data->log_file.is_open()) {
                file->log_data->log_file << message;
                file->log_data->log_file.flush();
            }
            lock.unlock();
        }

        void Set(axiom::LogLevel level, std::string message){
            std::stringstream ss; 
            ss << timer::Current() << " - ";
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
            Save(log_line);
        }
        
        void Check(bool b, std::string message){
            if(!b) Set(LogLevel::ERROR ,"Failed When " + message);
            else Set(LogLevel::INFO , message + " was a success!");
        }
    }
}