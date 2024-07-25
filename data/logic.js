let ctx = 0;														// Canvas context
let canvas = 0;														// Canvas object
let lastAngle = 0;													// Last received angle
let trackCount = 0;													// Count of tracks used
let trackAngle = [-1.0];											// List of used tracks angles
let imageOffsetAngle = 0;											// Image offset angle (degrees)
let degreesPerStep = 0;												// Stepper degrees per step
let stepperReduction = 0;											// Stepper reduction
let microStepsPerStep = 0;											// Stepper micro steps per step
let radius = 0;														// Radius of circles (in percentage of image)
let startEventsPending = false;										// Do we have a start event pending ?
let startEventsDelay = 0;											// Event delay before trying to reconnect
let inSetup = false;												// Are we in setup.htm?
let img2 = 0;														// Center turntable image
let traceJava = false;												// Trace this java code
let messagesExtended = false;										// Is messages div extended?
let enableCircles = false;											// Are track circles enabled ton user view?

// Display message on console and messages HTML element
function showMessage(message, displayOnBrowser = true) {
	console.log(message);											// Display message on console
	let messages = document.getElementById('messages');				// Messages from HTML document
	if (messages && displayOnBrowser) {								// If messages exists in document
		let messageElem = document.createElement('div');			// Create a div element
		messageElem.textContent = message;							// Set message into
		messages.prepend(messageElem);								// Add it at top of messages
	}
}

// Loads setting from JSON file. Loads html fields if loadHtml is true.
function loadSettings(loadHtml) {
	trackAngle = [-1.0];
	const req = new XMLHttpRequest();
	// Executed when request state change
	req.onreadystatechange = function() {
		// Check for end of request and status 200 (Ok)
		if (req.readyState === 4 && req.status == "200") {
			// Load JSON data
			jsonData = req.response;
			// Analyze all items in JSON file
			for (const key in jsonData){
				// If we shoud load data
				if (loadHtml) {
					// Find element giving key in document
					if (document.getElementById(key) != null) {
						// For axx elements
						if (key.substring(0,1) == "a" && key.length <= 3) {
							// Load angle
							document.getElementById(key).value = roundOf(jsonData[key],2)+"°";
						// For trace and enable elements (checkboxes)
						} else if (key.substring(0,5) == "trace" || key.substring(0,6) == "enable" || key.substring(0,3) == "led") {
							document.getElementById(key).checked = (String(jsonData[key]).toLowerCase() == "true");
						// For other elements
						} else {
							dataValue = jsonData[key];
							// If content is number but not integer (float)
							if (typeof(dataValue) == "number" && !Number.isInteger(dataValue)) {
								// Round it to 2 decimals
								dataValue = roundOf(dataValue, 2);
							}
							// Extract element type
							dataType = document.getElementById(key).nodeName;
							// For "input" types, load value
							if (dataType == "INPUT") {
								document.getElementById(key).value = dataValue;
							} else {
							//	Else, load text
								document.getElementById(key).innerText = dataValue;
							}
						}
					} else {
						// There's no elements with that key in document
						showMessage("### Can't find "+key+" in document ###");
					}
				}
				// If element is "trackCount"
				if (key == "trackCount") {
					// Keep it in a variable
					trackCount = jsonData[key];
					// If we asked for html update
					if (loadHtml) {
						for (let i = 1; i <= 36; i++) {
							// Display only elements up to track count
							document.getElementById("l"+i).hidden = (i > trackCount);
						}
					}
				}
				// If element is axx
				if (key.substring(0,1) == "a" && key.length <= 3) {
					// Add track position in trackAngle table
					trackAngle.push(jsonData[key]);
				}
				// If element is "radius"
				if (key == "radius") {
					// Save it into variable
					radius = parseFloat(jsonData[key]);
				}
				// Same for "imageOffsetAngle" and others
				if (key == "imageOffsetAngle") {
					imageOffsetAngle = parseFloat(jsonData[key]);
				}
				if (key == "degreesPerStep") {
					degreesPerStep = parseFloat(jsonData[key]);
				}
				if (key == "stepperReduction") {
					stepperReduction = parseFloat(jsonData[key]);
				}
				if (key == "microStepsPerStep") {
					microStepsPerStep = parseFloat(jsonData[key]);
				}
				if (key == "traceJava") {
					traceJava = jsonData[key] == true;
				}
				if (key == "enableCircles") {
					enableCircles = jsonData[key] == true;
				}
			};
			// Reload image with new values
			turn(lastAngle);
		}
		// Trace end of request with error
		if (req.readyState === 4 && req.status != "200") {
			showMessage(req.responseURL+" returned "+req.responseText);
		}
	}
	// Ask for JSON file nammed settings.json
	req.responseType = "json";
	req.open("GET", "settings.json");
	req.send();
}

function loadCanvas() {
	// Load canvas from document
	canvas = document.getElementById("canvas");
	// If ok
	if (canvas.getContext) {
		// Set 2D context
		ctx = canvas.getContext("2d");
		// Create new image
		let img = new Image();
		// Executed when image loaded
		img.onload = function () {
			// Save image width and height
			ctx.canvas.width = img.width;
			ctx.canvas.height = img.height;
			ctx.drawImage(img, 0, 0);
			if (inSetup) {												// If in setup
				// Rotate to last given angle
				turn(lastAngle);
			}
		};

		// Image to be loaded
		img.src = "imagePont.png";
		// Center turntable image
		img2 = new Image();
		img2.src = "plaque.png";
		// Executed when image loaded
		img2.onload = function () {
		// Rotate to last given angle
		turn(lastAngle);
		};
	}
}

// Draw circles around turntable, with number inside
function drawCircles() {
	const w = 32;													// Circles width
	const imgWidth = ctx.canvas.width;								// Image desired width & height
	const imgHeight = ctx.canvas.height
	ctx.save();														// Save context
	for (let i = 1; i <= trackCount; i++) {							// For all used tracks
		const angle = parseFloat(trackAngle[i]);					// Get angle of track
		// Compute x & y giving angle and image size
		const x = (radius / 100.0 * Math.cos(angle * Math.PI / 180.0) * imgWidth) + (imgWidth / 2.0);
		const y = (-radius / 100.0 * Math.sin(angle * Math.PI / 180.0) * imgHeight) + (imgHeight / 2.0);
		ctx.beginPath();											// Start a path
		ctx.fillStyle = "rgb(200,0,0)";								// Set color to red
		ctx.arc(x, y, w/2, 0, 2.0 * Math.PI);						// Draw a circle pn x/y
		ctx.fill();													// Fill with red
		ctx.stroke();												// Stroke
		ctx.font = (w/2)+'pt Calibri';								// Compute font size and name
		ctx.fillStyle = 'white';									// Set color to white
		ctx.textAlign = 'center';									// Text alignment = center
		ctx.fillText(i, x, y+(w/4));								// Write index at right position
	}
	ctx.restore();													// Restore context
}

// Init all thinks, with a flag to tell if we're in setup or not
function initAll(inSetupContext) {
	inSetup = inSetupContext;
	loadCanvas();
	loadSettings(inSetup);
	initMessages();
	startEvents();
}

// Init message <div>
function initMessages() {
	const divMessage = document.getElementById("messages");														// Look for "message" div
	if (divMessage) {
		divMessage.ondblclick = function () {																	// Called when double click on messages div
			messagesExtended = !messagesExtended;																// Invert message extended
			document.getElementById("messages").className = (messagesExtended ? "largeScroll" : "smallScroll"); // Set heigth 10 times initial one if extended
		};
	}
}

// Executed when clicking on image to send a request to webserver
function sendLocationPct(event) {
	const img = ctx.canvas;											  // Get canvas
	// Return x & y percentage (to allow image resizing) of cursor when clicked
	const pct_x = Math.round((event.clientX - img.offsetLeft) * 1000 / img.clientWidth) / 10;
	const pct_y = Math.round((event.clientY - img.offsetTop) * 1000 / img.clientHeight) / 10;
	data = pct_x + "/" + pct_y
	// Send the request. Webserver will move to right track if click is close to a track
	const req = new XMLHttpRequest();
	req.open("GET", location.origin+'/pos/'+data);
	req.onreadystatechange = function() {
		// Trace end of request with error
		if (req.readyState === 4 && req.status != "200") {
			showMessage(req.responseURL+" returned "+req.responseText);
		}
	}
	req.send();
}

// Connect events and define associated functions
function startEvents(){
	const es = new EventSource('/events');							// Connect
	startEventsPending = false;										// Clear start pending
	es.onopen = function(e) {										// Executed when connected
		showMessage("# Events opened #", traceJava);				// Just log
		startEventsDelay = 0;										// We tried to open, reset start delay
	};
	es.onerror = function(e) {										// Executed in case of error
		if (e.target.readyState == EventSource.CLOSED) {			// If state is "closed"
			showMessage("### Events closed ("+e.target.readyState+") ###"); // Log
			if (!startEventsPending) {								// If no start pending
				setTimeout(startEvents, startEventsDelay * 1000);	// Ask for a delayed start 
				startEventsPending = true;							// Set start pending
				if (startEventsDelay < 10) {						// Increase delay up to 10 sec
					startEventsDelay++;
				}
			}
		}
	};
	es.addEventListener('angle', (e) => {							// Executed when receiving an "angle" event
		showMessage("# Angle: "+ e.data+" #", traceJava);			// Log
		lastAngle = parseFloat(e.data);								// Extract value as float
		const angleDisplay = document.getElementById("angle");		// Locate id="angle"
		if (angleDisplay) {											// If found
			angleDisplay.innerText = roundOf(lastAngle,2);			// Set value, rounded to 2 decimals
		}
		const angleError = document.getElementById("angleError");	// Locate id="angleError"
		if (angleError) {											// If found
			const angError = degreesPerStep * stepperReduction / (microStepsPerStep * 2);
			angleError.innerText = roundOf(angError,2);				// Set value, rounded to 2 decimals
		}
		turn(lastAngle);											// Display table to the required angle
	});
	es.addEventListener('settings', (e) => {						// Executed when receiving a "settings" event
		if (e.data) {
		  console.log("Settings: "+ e.data);						// Log
		  loadSettings(inSetup);									// Reload setting giving saved inSetup flag
		}
	});
	es.addEventListener('info', (e) => {							// Executed when receiving an "info" event
		if (e.data) {
			showMessage(e.data)										// Set message error
		}
	});
	es.addEventListener('error', (e) => {							// Executed when receiving an "error" event
		if (e.data) {
		  showMessage("*** "+e.data+" ***");						// Set message error
		}
	});
}

// Display center of turntable, in center of image, giving required rotation
function turn(angle) {
	angle -= imageOffsetAngle;										// Fix angle using imageOffset
	ctx.save();													// Save context
	ctx.translate((ctx.canvas.width/2), (ctx.canvas.height/2)); // Seet center to center of image
	ctx.rotate((Math.PI / 180) * -angle);						// Rotate image as requested
	ctx.translate(-(ctx.canvas.width/2), -(ctx.canvas.height/2));// Come back to origin and draw image
	ctx.drawImage(img2, canvas.width/2 - img2.width/2, canvas.height/2 -img2.height/2, img2.width, img2.height);
	ctx.restore();												// Restore context
	if (inSetup || enableCircles) {								// If in setup or enableCircle is true
		drawCircles();											// Display circles
	}
}

// Executed when a "test sound" button is pushed. Index contains 1, 2 or 3 for before/during/after move sound test
function testSound(index){
	const req = new XMLHttpRequest();
	req.open("GET", location.origin+'/tstsnd/'+index);
	req.onreadystatechange = function() {
		// Trace end of request with error
		if (req.readyState === 4 && req.status != "200") {
			showMessage(req.responseURL+" returned "+req.responseText);
		}
	}
	req.send();
}

// Executed to request a move of a given (signed) count of steps
function move(steps){
	const req = new XMLHttpRequest();
	req.open("GET", location.origin+'/move/'+steps);
	req.onreadystatechange = function() {
		// Trace end of request with error
		if (req.readyState === 4 && req.status != "200") {
			showMessage(req.responseURL+" returned "+req.responseText);
		}
	}
	req.send();
}

// Executed to request setting center of turntable image reference rotation
function setImageReference() {
	const req = new XMLHttpRequest();
	req.open("GET", location.origin+'/imgref');
	req.onreadystatechange = function() {
		// Trace end of request with error
		if (req.readyState === 4 && req.status != "200") {
			showMessage(req.responseURL+" returned "+req.responseText);
		}
	}
	req.send();
}

// Executed to request a move to a given track
function moveToTrack(track) {
	const req = new XMLHttpRequest();
	req.open("GET", location.origin+'/goto/'+track);
	req.onreadystatechange = function() {
		// Trace end of request with error
		if (req.readyState === 4 && req.status != "200") {
			showMessage(req.responseURL+" returned "+req.responseText);
		}
	}
	req.send();
}

// Executed to request setting a track angle
function setAngle(line) {
	showMessage("# Set angle "+line+" #", traceJava);
	const req = new XMLHttpRequest();
	req.open("GET", location.origin+'/setangle/'+line);
	req.onreadystatechange = function() {
		// Trace end of request with error
		if (req.readyState === 4 && req.status != "200") {
			showMessage(req.responseURL+" returned "+req.responseText);
		}
	}
	req.send();
}

// Executed when a value is changed by user
function changed(object) {
	showMessage("# Set changed "+object.id+" #", traceJava);
	const req = new XMLHttpRequest();
	if (object.id.substring(0,5) == "trace" || object.id.substring(0,6) == "enable" || object.id.substring(0,3) == "led") {
		req.open("GET", location.origin+'/changed/'+object.id+"/"+object.checked);		// .checked is send for checkboxes
	} else {
		req.open("GET", location.origin+'/changed/'+object.id+"/"+object.value);		// .value else
	}
	req.onreadystatechange = function() {
		// Trace end of request with error
		if (req.readyState === 4 && req.status != "200") {
			showMessage(req.responseURL+" returned "+req.responseText);
		}
	}
	req.send();
}

// Rounds a float f to p decimals
function roundOf(n, p) {
	const n1 = n * Math.pow(10, p + 1);
	const n2 = Math.floor(n1 / 10);
	if (n1 >= (n2 * 10 + 5)) {
		return (n2 + 1) / Math.pow(10, p);
	}
	return n2 / Math.pow(10, p);
}