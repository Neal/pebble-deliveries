$(document).bind('pageinit', function() {

	var packages = [];
	var panels = 0;

	var data = /data=([^&|^\/]*)/.exec(location.search);
	if (data && data[1]) {
		var d = JSON.parse(decodeURIComponent(data[1]));
		packages = d.packages;
	}

	$.each(packages, function(index, pkg) {
		var newPanel = $('.panel-template').first().clone();
		addNewPanel(pkg.item_name, newPanel);
		$.each(pkg, function(index, value) {
			newPanel.find('#'+index).val(value);
		});
	});

	function addNewPanel(title, newPanel) {
		newPanel.find('.accordion-toggle').attr('href', '#panel' + (++panels));
		newPanel.find('.panel-collapse').attr('id', 'panel' + panels);
		newPanel.find('.panel-title').text(title);
		$('#accordion').append(newPanel.fadeIn());
		newPanel.find('#itemName').change(function() {
			newPanel.find('.panel-title').text($(this).val());
		});
		newPanel.find('.btn-delete').click(function() {
			newPanel.remove();
		});
		newPanel.find('.panel-heading').on('swipe', function() {
			if(newPanel.find('.btn-delete').is(':hidden')) { newPanel.find('.btn-delete').show(); } else { newPanel.find('.btn-delete').hide(); }
		});
	}

	$('.btn-add-package').on('click', function() {
		addNewPanel('New Package', $('.panel-template').first().clone());
	});

	$('.btn-save').on('click', function() {
		var packages = $.map($('.panel'), function(e) {
			if ($(e).find('.panel-collapse').attr('id').indexOf('template') != -1)
				return;
			var pkg = {};
			$(e).find('input').map(function() {
				pkg[this.id] = $(this).val();
			});
			return pkg;
		});
		var ret = {
			packages: packages
		};
		document.location = 'pebblejs://close#' + encodeURIComponent(JSON.stringify(ret));
	});

	$('.panel-container').sortable({
		handle: '.handle',
		axis: 'y'
	});
});
