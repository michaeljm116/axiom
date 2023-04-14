#include <iostream>
#include <string>
#include <mutex>

namespace axiom{
class Logger {
 public:
  enum class Severity {
    Debug,
    Info,
    Warning,
    Error
  };

  static Logger& GetInstance() {
    static Logger instance;
    return instance;
  }

  void Log(Severity severity, const std::string& message) {
    std::unique_lock<std::mutex> lock(mutex_);
    switch (severity) {
      case Severity::Debug:
        std::cout << "[DEBUG]: ";
        break;
      case Severity::Info:
        std::cout << "[INFO]: ";
        break;
      case Severity::Warning:
        std::cout << "[WARNING]: ";
        break;
      case Severity::Error:
        std::cout << "[ERROR]: ";
        break;
    }
    std::cout << message << std::endl;
  }

 private:
  Logger() = default;
  std::mutex mutex_;
};

inline void LogDebug(const std::string& message) {
  Logger::GetInstance().Log(Logger::Severity::Debug, message);
}

inline void LogInfo(const std::string& message) {
  Logger::GetInstance().Log(Logger::Severity::Info, message);
}

inline void LogWarning(const std::string& message) {
  Logger::GetInstance().Log(Logger::Severity::Warning, message);
}

inline void LogError(const std::string& message) {
  Logger::GetInstance().Log(Logger::Severity::Error, message);
}


}