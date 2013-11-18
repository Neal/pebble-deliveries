var maxTriesForSendingAppMessage = 3;
var timeoutForAppMessageRetry = 3000;
var timeoutForAPIRequest = 12000;

var apiKey = localStorage.getItem('api_key') || '';
var trackingNumber = localStorage.getItem('tracking_number') || '';
var itemName = localStorage.getItem('item_name') || trackingNumber;

function sendAppMessage(message, numTries, transactionId) {
	numTries = numTries || 0;
	if (numTries < maxTriesForSendingAppMessage) {
		numTries++;
		console.log('Sending AppMessage to Pebble: ' + JSON.stringify(message));
		Pebble.sendAppMessage(
			message, function() {}, function(e) {
				console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
				setTimeout(function() {
					sendAppMessage(message, numTries, e.data.transactionId);
				}, timeoutForAppMessageRetry);
			}
		);
	} else {
		console.log('Failed sending AppMessage for transactionId:' + transactionId + '. Bailing. ' + JSON.stringify(message));
	}
}

function makeRequest() {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'http://api.boxoh.com/v2/rest/key/' + apiKey + '/track/' + trackingNumber, true);
	xhr.timeout = timeoutForAPIRequest;
	xhr.onload = function(e) {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				res = JSON.parse(xhr.responseText);
				if (res.result == 'OK') {
					var description = '';
					var time = '';
					var location = '';
					var shipper = res.data.shipper || '';
					var deliveryEstimate = res.data.deliveryEstimate || shipper;

					if (res.data.tracking && res.data.tracking[0] && res.data.tracking[0].desc)
						description = res.data.tracking[0].desc;

					if (res.data.tracking && res.data.tracking[0] && res.data.tracking[0].time)
						time = res.data.tracking[0].time;

					if (res.data.tracking && res.data.tracking[0] && res.data.tracking[0].locStr)
						location = res.data.tracking[0].locStr;

					sendAppMessage({
						'item_name': itemName.substring(0,13),
						'description': description,
						'time': time,
						'location': location,
						'deliveryEstimate': deliveryEstimate
					});
				} else {
					console.log('Error received from Boxoh: [' + res.error.errorCode + '] ' + res.error.errorMessage);
					sendAppMessage({'item_name': res.error.errorMessage});
				}
			} else {
				console.log('Request returned error code ' + xhr.status.toString());
				sendAppMessage({'item_name': 'Error: ' + xhr.statusText});
			}
		}
	}
	xhr.ontimeout = function() {
		console.log('Error: request timed out!');
		sendAppMessage({'item_name': 'Error: Request timed out!'});
	};
	xhr.onerror = function(e) {
		console.log(JSON.stringify(e));
		sendAppMessage({'item_name': 'Error: Failed to connect!'});
	};
	xhr.send(null);
}

Pebble.addEventListener('ready', function(e) {});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received from Pebble: ' + JSON.stringify(e.payload));

	if (!apiKey) {
		console.log('API Key not set!');
		sendAppMessage({'item_name': 'Set Boxoh API key via Pebble app!'});
		return;
	}

	if (!trackingNumber) {
		console.log('tracking number not set!');
		sendAppMessage({'item_name': 'Set tracking number via Pebble app!'});
		return;
	}

	makeRequest();
});

Pebble.addEventListener('showConfiguration', function(e) {
	var uri = 'https://rawgithub.com/Neal/pebble-package-trackr/master/html/configuration.html?' +
				'api_key=' + encodeURIComponent(apiKey) +
				'&tracking_number=' + encodeURIComponent(trackingNumber) +
				'&item_name=' + encodeURIComponent(itemName);
	console.log('showing configuration at uri: ' + uri);
	Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
	console.log('configuration closed');
	if (e.response) {
		var options = JSON.parse(decodeURIComponent(e.response));
		console.log('options received from configuration: ' + JSON.stringify(options));
		apiKey = options['api_key'];
		trackingNumber = options['tracking_number'];
		itemName = options['item_name'] || trackingNumber;
		localStorage.setItem('api_key', apiKey);
		localStorage.setItem('tracking_number', trackingNumber);
		localStorage.setItem('item_name', itemName);
		makeRequest();
	} else {
		console.log('no options received');
	}
});
