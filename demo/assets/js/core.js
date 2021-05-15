/*
*  	MIT License
*	Copyright (c) 2018 石固

			
				  / \/\/\/`
				/ \///\/\/\`\`
				 /-\//\/ --\\
				 c| ⊙    ⊙ |]   
				  |   <  〃 |
				  \ _/\_   /
				   \ _=_  /
*	
*/

if (typeof data === 'undefined' || typeof data !== 'object')
	console.error('Can not load data, check your data file: data.js');
else {

	var root = document.createElement('ul');
	$('nav#menu').append(root);
	recursiveFolder(root, data, './notes');
}


/*
*   render file list
*/
function recursiveFolder(f, folderArr, rel) {

	// empty folder
	var len = folderArr.length;
	if (len === 0)
		return;

	for (var i = 0; i < len; i++) {

		var n = folderArr[i].text, 
			nxtTarget = folderArr[i].nodes;

		if (typeof nxtTarget === 'object') {

			// sub folder
			var li = document.createElement('li'),
				span = document.createElement('span'),
				ul = document.createElement('ul'),

				nRel;

			span.innerText = n;
			f.append(li);
			li.append(span);
			li.append(ul);

			nRel = rel + '/' + n;

			recursiveFolder(ul, nxtTarget, nRel);
		} else {

			var cover = 'welcome.html'; 
			if (rel === './notes' && n === cover)
				continue;

			// file
			var li = document.createElement('li');
			li.innerHTML = '<a href="' + rel + '/' + n + '" target="notesFrame">' + n.substr(0, n.length - 5) + '</a>';

			f.append(li);
		}
	}
}

/*
*   init plugin
*/
var $menu = $('nav#menu');
$menu.mmenu({

	extensions: [ 'theme-dark' ], // theme-light(Default)|theme-dark|theme-white|theme-black
	setSelected: true,
/*
	counters: true,
*/
	searchfield: {
		placeholder: 'Search notes'
	},
	iconbar: {
		add: true,
		size: 40,
		top: [
			'<a href=""><span class="icon-home"></span></a>',
			'<a href="https://github.com/SiGool/mkdnotes" target="_blank"><span class="icon-github"></span></a>'
		]
	},
	sidebar: {
		collapsed: {
			use: true,
		},
		expanded: {
			use: '(min-width: 992px)'
		}
	},
	navbars: [
		{
			content: ['searchfield']
		},
		{
			content: ['prev', 'breadcrumbs', 'close']
		}
	]
}, {
	navbars: {
		breadcrumbs: {
			removeFirst: true
		}
	}
});


// auto open and close
var menuApi = $menu.data('mmenu');

$('.mm-menu__blocker').css('z-index', '-1');

$('.mm-iconbar').on('click mouseenter', function() {

	if (!$menu.hasClass('mm-menu_opened'))
		menuApi.open();
});

$menu.mouseleave(function() {

	if ($menu.hasClass('mm-menu_opened'))
		menuApi.close();
});