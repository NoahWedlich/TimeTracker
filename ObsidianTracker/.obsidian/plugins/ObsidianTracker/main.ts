import { Notice, Plugin, requestUrl, RequestUrlParam, RequestUrlResponse } from 'obsidian';

export default class ObsidianTracker extends Plugin {
	async onload() {
		await this.loadSettings();
		
		const vault_name: string = this.app.vault.getName();
		
		const params: RequestUrlParam = {
			url: "http://localhost:7138",
			method: "POST",
			body: "TTE:Obsidian:" + vault_name + ":"	
		};
		
		requestUrl(params).then((response: RequestUrlResponse) => {
			if (response.text !== "VALID") {
				new Notice("Unable to connect to the RTT: Invalid response '" + response.text + "'");
			} else {
				new Notice("Successfully connected to the RTT: " + vault_name);
			}
		}).catch((error) => {
			new Notice("Unable to connect to the RTT: " + error);
		});
	}

	onunload() {
	}

	async loadSettings() {
	}

	async saveSettings() {
	}
}