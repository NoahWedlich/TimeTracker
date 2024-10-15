from typing import Union
import io

from Logger import Logger

class TTRFile:
    domains: list[str]
    entities: list[tuple[int, str]]
    
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
    
    ## Domain handling
    
    def domain_exists(self, domain: str) -> bool:
        if not self.ready():
            Logger.log_error("Invalid state")
            return False
            
        return domain in self.domains
    
    def domain_id_exists(self, domain_id: int) -> bool:
        if not self.ready():
            Logger.log_error("Invalid state")
            return False
            
        return domain_id < len(self.domains)
    
    def get_domain_id(self, domain: str) -> Union[int, None]:
        if not self.ready():
            Logger.log_error("Invalid state")
            return None
            
        if not self.domain_exists(domain):
            Logger.log_error("Domain does not exist: {}", domain)
            return None
            
        return self.domains.index(domain)
    
    def get_domain(self, domain_id: int) -> Union[str, None]:
        if not self.ready():
            Logger.log_error("Invalid state")
            return None
            
        if not self.domain_id_exists(domain_id):
            Logger.log_error("Domain ID does not exist: {}", domain_id)
            return None
            
        return self.domains[domain_id]
    
    
    ## Entity handling
    
    def entity_exists(self, entity: str, domain_id: int) -> bool:
        if not self.ready():
            Logger.log_error("Invalid state")
            return False
            
        return (domain_id, entity) in self.entities
    
    def entity_id_exists(self, entity_id: int) -> bool:
        if not self.ready():
            Logger.log_error("Invalid state")
            return False
            
        return entity_id < len(self.entities)     
    
    def get_entity_id(self, entity: str, domain_id: int) -> Union[int, None]:
        if not self.ready():
            Logger.log_error("Invalid state")
            return None
            
        if not self.entity_exists(entity, domain_id):
            Logger.log_error("Entity does not exist: {}", entity)
            return None
            
        return self.entities.index((domain_id, entity))
    
    def get_entity(self, entity_id: int) -> Union[tuple[int, str], None]:
        if not self.ready():
            Logger.log_error("Invalid state")
            return None
            
        if not self.entity_id_exists(entity_id):
            Logger.log_error("Entity ID does not exist: {}", entity_id)
            return None
            
        return self.entities[entity_id]
    
    
    ## File handling
    
    __path: str
    __file: io.BufferedReader
    
    __has_parsed: bool
    
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
            
        self.domains = []
        self.entities = []
        
        try:
            self.__file.seek(0)
            header: str = self.__file.read(3).decode("utf-8")
            
            if header != "TTR":
                Logger.log_error("Invalid file header: {}", header)
                return False
                
            offset_to_domains_start: int = int.from_bytes(self.__file.read(4), byteorder="little")
            offset_to_domains_end: int = int.from_bytes(self.__file.read(4), byteorder="little")
            
            offset_to_entities: int = int.from_bytes(self.__file.read(4), byteorder="little")
            
            self.__file.seek(offset_to_domains_start)
            
            num_of_domains: int = int.from_bytes(self.__file.read(1), byteorder="little")
            
            for i in range(num_of_domains):
                length: int = int.from_bytes(self.__file.read(1), byteorder="little")
                domain: str = self.__file.read(length).decode("utf-8")
                self.domains.append(domain)
                
            self.__file.seek(offset_to_entities)
            
            num_of_entities: int = int.from_bytes(self.__file.read(2), byteorder="little")
            
            for i in range(num_of_entities):
                domain: int = int.from_bytes(self.__file.read(1), byteorder="little")
                length: int = int.from_bytes(self.__file.read(1), byteorder="little")
                entity: str = self.__file.read(length).decode("utf-8")
                self.entities.append((domain, entity))
                
            self.__has_parsed = True
            return True
            
        except Exception as e:
            Logger.log_error("An error occurred while parsing: {}", e)
            return False
        