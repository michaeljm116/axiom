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
                    Set(log::Level::INFO, "Log File Opened");
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
                    if(l.lvl == log::Level::CHECK)
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

        void Set(axiom::log::Level level, std::string message){
            std::stringstream ss; 
            ss << timer::Current() << " - ";
            switch (level) {
                case log::Level::DEBUG:
                    ss << "[DEBUG] ";
                    break;
                case log::Level::INFO:
                    ss << "[INFO] ";
                    break;
                case log::Level::WARNING:
                    ss << "[WARNING] ";
                    break;
                case log::Level::ERROR:
                    ss << "[ERROR] ";
                    break;
                case log::Level::CHECK:
                    break;
            }
            ss << message << std::endl;
            std::string log_line = ss.str();
            std::cout << log_line;
            Save(log_line);
        }
        
        void Check(bool b, std::string message){
            if(!b) Set(log::Level::ERROR ,"Failed When " + message);
            else Set(log::Level::INFO , message + " was a success!");
        }
    }
}