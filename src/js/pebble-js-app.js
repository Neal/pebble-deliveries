var packages = JSON.parse(localStorage.getItem('packages')) || [];

var appMessageQueue = {
	queue: [],
	numTries: 0,
	maxTries: 5,
	add: function(obj) {
		this.queue.push(obj);
	},
	clear: function() {
		this.queue = [];
	},
	isEmpty: function() {
		return this.queue.length === 0;
	},
	nextMessage: function() {
		return this.isEmpty() ? {} : this.queue[0];
	},
	send: function() {
		if (this.queue.length > 0) {
			var ack = function() {
				appMessageQueue.numTries = 0;
				appMessageQueue.queue.shift();
				appMessageQueue.send();
			};
			var nack = function() {
				appMessageQueue.numTries++;
				appMessageQueue.send();
			};
			if (this.numTries >= this.maxTries) {
				console.log('Failed sending AppMessage: ' + JSON.stringify(this.nextMessage()));
				ack();
			}
			Pebble.sendAppMessage(this.nextMessage(), ack, nack);
		}
	}
};

function sendPackageList() {
	appMessageQueue.clear();
	if (packages.length === 0) {
		appMessageQueue.add({index: true});
	}
	for (var i = 0; i < packages.length; i++) {
		appMessageQueue.add({index: i, title: packages[i].itemName, subtitle: packages[i].trackingNumber});
	}
	appMessageQueue.send();
}

function sendPackageStatus(pkg) {
	appMessageQueue.clear();
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'http://api.boxoh.com/v2/rest/key/jqyxm3q354-1/track/' + pkg.trackingNumber, true);
	xhr.onload = function(e) {
		if (xhr.readyState == 4 && xhr.status == 200) {
			res = JSON.parse(xhr.responseText);
			if (res.result == 'OK') {
				if (res.data.tracking && res.data.tracking.length > 0) {
					for (var i = 0; i < res.data.tracking.length; i++) {
						var title = res.data.tracking[i].desc + ' at ' + res.data.tracking[i].locStr + ' on ' + res.data.tracking[i].time;
						appMessageQueue.add({index: i, title: title, status: true});
					}
				} else {
					appMessageQueue.add({index: 0, title: 'No tracking data found.', status: true});
				}
			} else {
				appMessageQueue.add({index: 0, title: res.error.errorMessage, status: true});
			}
		}
		appMessageQueue.send();
	};
	xhr.send(null);
}

Pebble.addEventListener('ready', function(e) {
	sendPackageList();
});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received from Pebble: ' + JSON.stringify(e.payload));
	if (e.payload.status) {
		sendPackageStatus(packages[e.payload.index]);
	} else {
		sendPackageList();
	}
});

Pebble.addEventListener('showConfiguration', function(e) {
	var data = {packages: packages};
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
