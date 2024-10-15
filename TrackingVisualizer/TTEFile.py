from typing import Union
import ctypes as c
import io

from Logger import Logger

class Date:
    year: int
    month: int
    day: int
    
    def __init__(self, year: int, month: int, day: int):
        self.year = year
        self.month = month
        self.day = day
        
    def __str__(self):
        return f"{self.year}-{self.month}-{self.day}"
        
    @staticmethod
    def decode(encoded_date: c.c_short) -> 'Date':
        return Date((encoded_date.value >> 9) & 0x7F, (encoded_date.value >> 5) & 0x0F, encoded_date.value & 0x1F)
    
class Event:
    date: Date
    entity_id: int
    hour: int
    minute: int
    second: int
    
    def __init__(self, date: Date, entity_id: int, hour: int, minute: int, second: int):
        self.date = date
        self.entity_id = entity_id
        self.hour = hour
        self.minute = minute
        self.second = second
        
    def __str__(self):
        return f"{self.date} {self.hour}:{self.minute}:{self.second} {self.entity_id}"
        
    @staticmethod
    def decode(date: Date, encoded_event: c.c_long) -> 'Event':
        return Event(date, (encoded_event.value >> 17) & 0x7FFF, (encoded_event.value >> 12) & 0x1F, (encoded_event.value >> 6) & 0x3F, encoded_event.value & 0x3F)
    
class TTEFile:
    events: list[Event]
    
    def __init__(self, path: str):
        self.__path = path
        self.__file = None
        
        self.__has_parsed = False
        
        if not self.__open():
            Logger.append_info("Failed to open file: {}", path)
            return
            
        if not self.__parse():
            Logger.append_info("Failed to parse file: {}", path)
            return
            
        
            
    def __del__(self):
        if not self.__close():
            Logger.append_info("Failed to close file: {}", self.__path)
    
    def ready(self) -> bool:
        return self.__has_parsed
    
    
    ## Date handling
    
    def date_exists(self, date: Date) -> bool:
        for event in self.events:
            if event.date == date:
                return True
        return False
    
    def get_dates(self) -> list[Date]:
        dates: list[Date] = []
        
        for event in self.events:
            if event.date not in dates:
                dates.append(event.date)
                
        return dates
    
    
    ## Event handling
    
    def event_exists(self, event: Event) -> bool:
        return event in self.events
    
    def get_events(self) -> list[Event]:
        return self.events
    
    def get_events_for_date(self, date: Date) -> list[Event]:
        events: list[Event] = []
        
        for event in self.events:
            if event.date == date:
                events.append(event)
                
        return events
    
    def get_events_for_entity(self, entity_id: int) -> list[Event]:
        events: list[Event] = []
        
        for event in self.events:
            if event.entity_id == entity_id:
                events.append(event)
                
        return events
    
    
    ## File handling
    
    __path: str
    __file: io.BufferedReader
    
    __has_parsed: bool = False
    
    def __open(self) -> bool:
        try:
            self.__file = open(self.__path, "rb")
            return True
        except FileNotFoundError:
            Logger.log_error("File not found")
            return False
        except PermissionError:
            Logger.log_error("Permission denied")
            return False
        except:
            Logger.log_error("Unknown error")
            return False
    
    def __close(self) -> bool:
        if self.__file is None:
            Logger.log_warning("File already closed")
            return True
            
        try:
            self.__file.close()
            return True
        except:
            Logger.log_error("Unknown error")
            return False
    
    def __parse(self) -> bool:
        if not self.__file and not self.__open():
            Logger.append_info("Failed to parse file: {}", self.__path)
            return False
            
        self.events: list[Event] = []
        current_date: Union[Event, None] = None
        
        try:
            self.__file.seek(0)
            
            header: str = self.__file.read(3).decode("utf-8")
            
            if header != "TTE":
                Logger.log_error("Invalid file header: {}", header)
                return False
                
            num_of_dates: int = int.from_bytes(self.__file.read(2), byteorder="little")
            
            for i in range(num_of_dates):
                encoded_date: c.c_short = c.c_short(int.from_bytes(self.__file.read(2), byteorder="little"))
                current_date = Date.decode(encoded_date)
                
                num_of_events: int = int.from_bytes(self.__file.read(4), byteorder="little")
                
                for j in range(num_of_events):
                    encoded_event: c.c_long = c.c_long(int.from_bytes(self.__file.read(4), byteorder="little"))
                    event = Event.decode(current_date, encoded_event)
                    
                    # if event.entity_id == 0 and len(self.events) > 0:
                    #     last_event = self.events[-1]
                    #     emulated_shutdown_event = Event(last_event.date, 0, last_event.hour, last_event.minute, last_event.second)
                    #     self.events.append(emulated_shutdown_event)
                    #     
                    #     Logger.log_warning("Emulated shutdown event")
                    
                    self.events.append(event)
                    
            self.__has_parsed = True
            return True
            
        except Exception as e:
            Logger.log_error("An error occurred while parsing: {}", e)
            return False