from typing import Union
from enum import Enum
import time

class Logger:
    path: Union[str, None] = None
    
    class LogLevel(Enum):
        LOG_INFO = 1
        LOG_WARNING = 2
        LOG_ERROR = 3
        
    @staticmethod
    def set_file_path(path: str):
        Logger.path = path
        
    @staticmethod
    def emit_line(line: str):
        print(line)
        
        if Logger.path is not None:
            try:
                with open(Logger.path, "a") as file:
                    file.write(line + "\n")
            except FileNotFoundError:
                pass
    
    @staticmethod
    def log(level: LogLevel, message: str, *args):
        timestamp = time.strftime("(%Y-%m-%d %H:%M:%S)")
        
        if level == Logger.LogLevel.LOG_INFO:
            log_level = "INFO"
        elif level == Logger.LogLevel.LOG_WARNING:
            log_level = "WARNING"
        elif level == Logger.LogLevel.LOG_ERROR:
            log_level = "ERROR"
        else:
            log_level = "UNKNOWN"
            
        line = f"{timestamp} [{log_level}] {message.format(*args)}"
            
        Logger.emit_line(line)
        
            
    @staticmethod
    def log_info(message: str, *args):
        Logger.log(Logger.LogLevel.LOG_INFO, message, *args)
        
    @staticmethod
    def log_warning(message: str, *args):
        Logger.log(Logger.LogLevel.LOG_WARNING, message, *args)
        
    @staticmethod
    def log_error(message: str, *args):
        Logger.log(Logger.LogLevel.LOG_ERROR, message, *args)
        
    @staticmethod
    def append_info(message: str, *args):
        formated_message = message.format(*args)
        Logger.emit_line(" " * 24 + " - " + formated_message)