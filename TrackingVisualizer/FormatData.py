
from datetime import datetime
from typing import Union

from TTRFile import TTRFile
from TTEFile import TTEFile

from Logger import Logger

def format_data(path: str) -> list[tuple[str, str, datetime, datetime]]:
    ttr_file_path = path + ".ttr"
    tte_file_path = path + ".tte"
    
    ttr_file = TTRFile(ttr_file_path)
    tte_file = TTEFile(tte_file_path)
    
    current_events: dict[str, Union[tuple[str, datetime], None]] = {
        "Runtime": None,
        "System": None,
        "Activity": None,
        "Browser": None,
        "VSCode": None
    }
    
    events: list[tuple[str, str, datetime, datetime]] = []
    
    last_website = None
    last_vscode_project = None
    
    startup_event = tte_file.events[0]
    
    current_events["Runtime"] = ("Startup", datetime(2000 + startup_event.date.year, startup_event.date.month, startup_event.date.day, startup_event.hour, startup_event.minute, startup_event.second))
    
    for event in tte_file.events[1:]:
        domain_id, entity = ttr_file.get_entity(event.entity_id)
        domain = ttr_file.get_domain(domain_id)
        
        if domain is None or entity is None:
            Logger.log_error("Failed to get domain and entity")
            continue
        
        if domain == "Runtime":
            if current_events["Runtime"] is not None:
                if current_events["Runtime"][0] == "Startup":
                    events.append(("Runtime", "power off", current_events["Runtime"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                else:
                    events.append(("Runtime", current_events["Runtime"][0], current_events["Runtime"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["Runtime"] = None
            if current_events["System"] is not None:
                events.append(("System", current_events["System"][0], current_events["System"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["System"] = None
            if current_events["Activity"] is not None:
                events.append(("Activity", current_events["Activity"][0], current_events["Activity"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["Activity"] = None
            if current_events["Browser"] is not None:
                events.append(("Browser", current_events["Browser"][0], current_events["Browser"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["Browser"] = None
            if current_events["VSCode"] is not None:
                events.append(("VSCode", current_events["VSCode"][0], current_events["VSCode"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["VSCode"] = None
                
            current_events["Runtime"] = (entity, datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second))
        
        if domain == "System":
            if current_events["System"] is not None:
                events.append(("System", current_events["System"][0], current_events["System"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["System"] = None
            if current_events["Runtime"] is not None:
                events.append(("Runtime", current_events["Runtime"][0], current_events["Runtime"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["Runtime"] = None
            if current_events["Activity"] is not None:
                events.append(("Activity", current_events["Activity"][0], current_events["Activity"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["Activity"] = None
            if current_events["Browser"] is not None:
                events.append(("Browser", current_events["Browser"][0], current_events["Browser"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["Browser"] = None
            if current_events["VSCode"] is not None:
                events.append(("VSCode", current_events["VSCode"][0], current_events["VSCode"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["VSCode"] = None
                
            current_events["System"] = (entity, datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second))
            
            if entity == "chrome.exe":
                current_events["Browser"] = (last_website, datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second))
            elif entity == "Code.exe":
                current_events["VSCode"] = (last_vscode_project, datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second))
                
        elif domain == "Activity":
            if current_events["Activity"] is not None:
                events.append(("Activity", current_events["Activity"][0], current_events["Activity"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["Activity"] = None
            if current_events["System"] is not None:
                events.append(("System", current_events["System"][0], current_events["System"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["System"] = None
            if current_events["Runtime"] is not None:
                events.append(("Runtime", current_events["Runtime"][0], current_events["Runtime"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["Runtime"] = None
            if current_events["Browser"] is not None:
                events.append(("Browser", current_events["Browser"][0], current_events["Browser"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["Browser"] = None
            if current_events["VSCode"] is not None:
                events.append(("VSCode", current_events["VSCode"][0], current_events["VSCode"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["VSCode"] = None
                
            current_events["Activity"] = (entity, datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second))
            
        elif domain == "Browser":
            if current_events["Browser"] is not None:
                events.append(("Browser", current_events["Browser"][0], current_events["Browser"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["Browser"] = None
            
            current_events["Browser"] = (entity, datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second))
            last_website = entity
            
        elif domain == "VSCode":
            if current_events["VSCode"] is not None:
                events.append(("VSCode", current_events["VSCode"][0], current_events["VSCode"][1], datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second, 0)))
                current_events["VSCode"] = None
                
            current_events["VSCode"] = (entity, datetime(2000 + event.date.year, event.date.month, event.date.day, event.hour, event.minute, event.second))
            last_vscode_project = entity
            
            
    events = [event for event in events if event[0] != "Runtime" and event[0] != "Activity"] # Comment this line to include runtime events
    
    return events