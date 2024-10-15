
import plotly.express as px
import pandas as pd

from FormatData import format_data


def display_timeline(path: str):
    events = format_data(path)

    data = []
    
    for event in events:
        data.append({"Task": event[1], "Start": event[2], "Finish": event[3], "Resource": event[0]})
        
    df = pd.DataFrame(data)

    fig = px.timeline(df, x_start = "Start", x_end = "Finish", y = "Task", color = "Resource")
    fig.update_yaxes(autorange = "reversed")
    fig.show()