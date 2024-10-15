from Timeline import display_timeline
from Piechart import display_piechart

def main():
    display_piechart("../TimeTracker/x64/Release/TimeTracker")
    display_timeline("../TimeTracker/x64/Release/TimeTracker")
    
    # display_timeline("TimeTracker")
    

if __name__ == "__main__":
    main()