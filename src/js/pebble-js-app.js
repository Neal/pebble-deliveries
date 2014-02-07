var options = {
	appMessage: {
		maxTries: 3,
		retryTimeout: 3000,
		timeout: 100
	}
};

var appMessageQueue = [];
var packages = JSON.parse(localStorage.getItem('packages')) || [];

function sendAppMessageQueue() {
	if (appMessageQueue.length > 0) {
		currentAppMessage = appMessageQueue[0];
		currentAppMessage.numTries = currentAppMessage.numTries || 0;
		currentAppMessage.transactionId = currentAppMessage.transactionId || -1;
		if (currentAppMessage.numTries < options.appMessage.maxTries) {
			console.log('Sending AppMessage to Pebble: ' + JSON.stringify(currentAppMessage.message));
			Pebble.sendAppMessage(
				currentAppMessage.message,
				function(e) {
					appMessageQueue.shift();
					setTimeout(function() {
						sendAppMessageQueue();
					}, options.appMessage.timeout);
				}, function(e) {
					console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
					appMessageQueue[0].transactionId = e.data.transactionId;
					appMessageQueue[0].numTries++;
					setTimeout(function() {
						sendAppMessageQueue();
					}, options.appMessage.retryTimeout);
				}
			);
		} else {
			appMessageQueue.shift();
			console.log('Failed sending AppMessage for transactionId:' + currentAppMessage.transactionId + '. Bailing. ' + JSON.stringify(currentAppMessage.message));
		}
	} else {
		console.log('AppMessage queue is empty.');
	}
}

function sendPackageList() {
	appMessageQueue = [];
	if (packages.length === 0) {
		appMessageQueue.push({message: {index: true}});
	}
	for (var i = 0; i < packages.length; i++) {
		appMessageQueue.push({message: {index: i, title: packages[i].itemName, subtitle: packages[i].trackingNumber}});
	}
	sendAppMessageQueue();
}

function sendPackageStatus(pkg) {
	appMessageQueue = [];
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'http://api.boxoh.com/v2/rest/key/jqyxm3q354-1/track/' + pkg.trackingNumber, true);
	xhr.onload = function(e) {
		if (xhr.readyState == 4 && xhr.status == 200) {
			res = JSON.parse(xhr.responseText);
			if (res.result == 'OK') {
				if (res.data.tracking && res.data.tracking.length > 0) {
					for (var i = 0; i < res.data.tracking.length; i++) {
						var title = res.data.tracking[i].desc + ' at ' + res.data.tracking[i].locStr + ' on ' + res.data.tracking[i].time;
						appMessageQueue.push({message: {index: i, title: title, status: true}});
					}
				} else {
					appMessageQueue.push({message: {index: 0, title: 'No tracking data found.', status: true}});
				}
			} else {
				appMessageQueue.push({message: {index: 0, title: res.error.errorMessage, status: true}});
			}
		}
		sendAppMessageQueue();
	};
	xhr.send(null);
}

Pebble.addEventListener('ready', function(e) {});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received from Pebble: ' + JSON.stringify(e.payload));
	if (e.payload.status) {
		sendPackageStatus(packages[e.payload.index]);
	} else {
		sendPackageList();
	}
});

Pebble.addEventListener('showConfiguration', function(e) {
	var data = {
		packages: packages
	};
	var uri = 'http://neal.github.io/pebble-package-trackr/index.html?data=' + encodeURIComponent(JSON.stringify(data));
	console.log('showing configuration at uri: ' + uri);
	Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
	if (e.response) {
		var data = JSON.parse(decodeURIComponent(e.response)) || [];
		console.log('[configuration] data received: ' + JSON.stringify(data));
		packages = data.packages;
		localStorage.setItem('packages', JSON.stringify(packages));
		sendPackageList();
	}
});
