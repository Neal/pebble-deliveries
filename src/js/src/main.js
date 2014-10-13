var Deliveries = {
	packages: JSON.parse(localStorage.getItem('packages')) || [],

	init: function() {
		if (!(Deliveries.packages instanceof Array)) Deliveries.packages = [];
		Deliveries.sendPackages();
	},

	error: function(type, error) {
		appMessageQueue.send({type:type, method:METHOD.ERROR, title:error});
	},

	sendPackages: function() {
		if (!Deliveries.packages.length) {
			return Deliveries.error(TYPE.PACKAGE, 'No packages found. Use the app settings in the Pebble mobile app to add packages.');
		}
		appMessageQueue.send({type:TYPE.PACKAGE, method:METHOD.SIZE, index:Deliveries.packages.length});
		for (var i = 0; i < Deliveries.packages.length; i++) {
			appMessageQueue.send({type:TYPE.PACKAGE, method:METHOD.DATA, index:i, title:Deliveries.packages[i].itemName.substring(0,32), subtitle:Deliveries.packages[i].trackingNumber.substring(0,32)});
		}
	},

	sendProgress: function(trackingNumber) {
		appMessageQueue.clear();
		Deliveries.error(TYPE.PROGRESS, 'Requesting tracking information for ' + trackingNumber + '...');
		var xhr = new XMLHttpRequest();
		xhr.open('GET', 'http://www.packagetrackr.com/track/' + encodeURIComponent(trackingNumber) + '.json', true);
		xhr.onload = function() {
			var res = {};
			try { res = JSON.parse(xhr.responseText); } catch(e) { return Deliveries.error(TYPE.PROGRESS, 'No tracking data found.'); }

			if (res.track && res.track.eSTDeliveryDate && res.track.eSTDeliveryDate.display && res.track.eSTDeliveryDate.display.length) {
				appMessageQueue.send({type:TYPE.PROGRESS, method:METHOD.ESTIMATE, title:res.track.eSTDeliveryDate.display.substring(0,64)});
			}

			if (res.track && res.track.packageProgress && res.track.packageProgress.length) {
				var progress = res.track.packageProgress;
				appMessageQueue.send({type:TYPE.PROGRESS, method:METHOD.SIZE, index:progress.length});
				for (var i = 0; i < progress.length; i++) {
					var title = progress[i].description + ' at ' + progress[i].location.fullLocation + ' on ' + progress[i].processDate.display;
					appMessageQueue.send({type:TYPE.PROGRESS, method:METHOD.DATA, index:i, title:title.substring(0,144)});
				}
			} else {
				Deliveries.error(TYPE.PROGRESS, 'No tracking data found.');
			}
		};
		xhr.onerror = function() { Deliveries.error(TYPE.PROGRESS, 'Connection error!'); };
		xhr.ontimeout = function() { Deliveries.error(TYPE.PROGRESS, 'Connection timed out! Please try again later.');};
		xhr.timeout = 60000;
		xhr.send(null);
	},

	handleAppMessage: function(e) {
		console.log('AppMessage received: ' + JSON.stringify(e.payload));
		if (!e.payload.method) return;
		switch (e.payload.method) {
			case METHOD.REQUESTPACKAGES:
				Deliveries.sendPackages();
				break;
			case METHOD.REQUESTPROGRESS:
				Deliveries.sendProgress(Deliveries.packages[e.payload.index].trackingNumber);
				break;
		}
	},

	showConfiguration: function() {
		var data = {packages: Deliveries.packages};
		Pebble.openURL('https://ineal.me/pebble/deliveries/configuration/?data=' + encodeURIComponent(JSON.stringify(data)));
	},

	handleConfiguration: function(e) {
		console.log('configuration received: ' + JSON.stringify(e));
		if (!e.response) return;
		if (e.response === 'CANCELLED') return;
		var data = JSON.parse(decodeURIComponent(e.response));
		if (data.packages) {
			Deliveries.packages = data.packages;
			localStorage.setItem('packages', JSON.stringify(Deliveries.packages));
			Deliveries.sendPackages();
		}
	}
};

Pebble.addEventListener('ready', Deliveries.init);
Pebble.addEventListener('appmessage', Deliveries.handleAppMessage);
Pebble.addEventListener('showConfiguration', Deliveries.showConfiguration);
Pebble.addEventListener('webviewclosed', Deliveries.handleConfiguration);
