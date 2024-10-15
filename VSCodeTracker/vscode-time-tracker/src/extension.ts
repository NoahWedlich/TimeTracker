import * as vscode from 'vscode';

import('node-fetch').then((module) => {
	let fetch = module.default;
});

export function activate(context: vscode.ExtensionContext) {

	let workspacePaths = vscode.workspace.workspaceFolders;
	let workspace = "unknown";
	
	if (workspacePaths) {
		workspace = workspacePaths[0].name;
	}
	
	fetch(
		"http://localhost:7138", {
			method: "POST",
			body: "TTE:VSCode:" + workspace + ":"
		}
	).then(response => response.text())
	.then(data => {
		if (data !== "VALID") {
			vscode.window.showErrorMessage("Unable to connect to the RTT");
		}
	}).catch((error) => {
		vscode.window.showErrorMessage("Unable to connect to the RTT");
	});
}