function handleActivated(activeInfo) {
    id = activeInfo.tabId;
    chrome.tabs.get(id, (tab) => {
        handle_changed_tab(tab);
    });
}

function handleUpdated(_, changeInfo, tab) {
    if (changeInfo.url) {
        handle_changed_tab(tab);
    }
}

function handle_changed_tab(tab) {
    url = tab.url;
    if (url === "") {
        url = tab.pendingUrl;
    }
    
    if (url === "") {
        return;
    }
    
    let { hostname } = new URL(url);
    
    if (hostname.startsWith("www.")) {
        hostname = hostname.slice(4);
    }
    
    // const count = hostname.split(".").length;
    // 
    // switch (count) {
    //     case 1:
    //         break;
    //     case 2:
    //         hostname = hostname.split(".")[0];
    //         break;
    //     case 3:
    //         hostname = hostname.split(".")[1];
    //         break;
    //     default:
    //         console.log("Error: " + hostname);
    //         return;
    // }
    
    hostname = hostname.toLowerCase();
    
    fetch("http://localhost:7138/", {
        method: "POST",
        body: "TTE:Browser:" + hostname + ":",
    }).then((response) => {
        if (!response.ok) {
            console.log("Error: " + response.status);
        }
        
        response.text().then((text) => {
            if (text === "VALID") {
                console.log("Valid");
            } else {
                console.log("Error: " + text);
            }
        });
        
    });
}

chrome.tabs.onActivated.addListener(handleActivated);
chrome.tabs.onUpdated.addListener(handleUpdated);