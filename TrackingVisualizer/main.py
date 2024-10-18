from Timeline import display_timeline
from Piechart import display_piechart

def main():
    path = "C:\\Users\\Noah Wedlich\\AppData\\Roaming\\TimeTracker\\TimeTracker"
    
    display_piechart(path)
    display_timeline(path)
    
    # display_timeline("TimeTracker")
    

if __name__ == "__main__":
    main()