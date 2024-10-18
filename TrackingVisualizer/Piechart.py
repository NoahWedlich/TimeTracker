
from datetime import timedelta

import plotly.graph_objects as go
from plotly.subplots import make_subplots

from FormatData import format_data

def seconds_to_time(seconds: int) -> str:
    hours = int(seconds // 3600)
    minutes = int((seconds % 3600) // 60)
    seconds = int(seconds % 60)
    
    return f"{hours:02}:{minutes:02}:{seconds:02}"

def display_piechart(path: str):
    events = format_data(path)
            
    applications: dict[str, timedelta] = {}
    websites: dict[str, timedelta] = {}
    vscode_projects: dict[str, timedelta] = {}
    obsidian_projects: dict[str, timedelta] = {}
    
    for event in events:
        duration = event[3] - event[2]
        
        if event[0] == "System":
            if event[1] in applications.keys():
                applications[event[1]] += duration
            else:
                applications[event[1]] = duration
        elif event[0] == "Browser":
            if event[1] in websites.keys():
                websites[event[1]] += duration
            else:
                websites[event[1]] = duration
        elif event[0] == "VSCode":
            if event[1] in vscode_projects.keys():
                vscode_projects[event[1]] += duration
            else:
                vscode_projects[event[1]] = duration
        elif event[0] == "Obsidian":
            if event[1] in obsidian_projects.keys():
                obsidian_projects[event[1]] += duration
            else:
                obsidian_projects[event[1]] = duration
    
    application_labels = []
    application_values = []
    
    for application, duration in applications.items():
        application_labels.append(application)
        application_values.append(duration.total_seconds())
        
    website_labels = []
    website_values = []
    
    for website, duration in websites.items():
        website_labels.append(website)
        website_values.append(duration.total_seconds())
    
    vscode_projects_labels = []
    vscode_projects_values = []
    
    for project, duration in vscode_projects.items():
        vscode_projects_labels.append(project)
        vscode_projects_values.append(duration.total_seconds())
        
    obsidian_projects_labels = []
    obsidian_projects_values = []
    
    for project, duration in obsidian_projects.items():
        obsidian_projects_labels.append(project)
        obsidian_projects_values.append(duration.total_seconds())
    
        
    combined_labels = application_labels + website_labels + vscode_projects_labels + obsidian_projects_labels
    combined_parents = ["" for _ in range(len(application_labels))] + ["chrome.exe" for _ in range(len(website_labels))] + ["Code.exe" for _ in range(len(vscode_projects_labels))] + ["Obsidian.exe" for _ in range(len(obsidian_projects_labels))]
    
    combined_values = application_values + website_values + vscode_projects_values + obsidian_projects_values
    combined_customdata = [seconds_to_time(value) for value in combined_values]
    
    total_duration = sum(combined_values)
    threshold = 0.004 * total_duration
    
    combined_labels = [(label if label != None else f"unknown_{value}") for label, value in zip(combined_labels, combined_values) if value > threshold]
    combined_parents = [parent for parent, value in zip(combined_parents, combined_values) if value > threshold]
    combined_customdata = [customdata for customdata, value in zip(combined_customdata, combined_values) if value > threshold]
    combined_values = [value for value in combined_values if value > threshold]
    
    fig = go.Figure(go.Sunburst(
        labels=combined_labels,
        parents=combined_parents,
        values=combined_values,
        customdata=combined_customdata,
        branchvalues="total",
        hovertemplate="<b>%{label}</b><br>%{customdata}<extra></extra>"
    ))
    
    fig.update_layout(margin=dict(t=0, l=0, r=0, b=0))
    
    fig.show()
    
    
    # fig = make_subplots(rows=1, cols=2, specs=[[{'type':'domain'}, {'type':'domain'}]])
    #     
    # fig.add_trace(go.Pie(labels=application_labels, values=application_values, name="Applications"), 1, 1)
    # 
    # fig.add_trace(go.Pie(labels=website_labels, values=website_values, name="Websites"), 1, 2)
    # 
    # fig.update_traces(hole=.4, hoverinfo="label+percent+name")
    # 
    # fig.update_layout(
    #     title_text="Time spent on applications and websites",
    #     annotations=[dict(text='Sys', x=sum(fig.get_subplot(1, 1).x) / 2, y=0.5, font_size=20, showarrow=False, xanchor='center'),
    #                  dict(text='Web', x=sum(fig.get_subplot(1, 2).x) / 2, y=0.5, font_size=20, showarrow=False, xanchor='center')]
    # )
    # 
    # fig.show()