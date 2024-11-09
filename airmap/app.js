const height = 2400;
const width = 4800;
const dotSize = 6;

const map = document.getElementById('map');
const modal = document.getElementById('modal');

map.style.width = width + 'px';
map.style.height = height + 'px';

map.addEventListener('click', e => {
	if (e.target != map) {
		return;
	}
	modal.style.display = 'none';
});

function createDot(lat, long) {
	// in plate carrée projection, x is long and lat is y. just need to
	// normalize
	x = (long + 180) * (width/360);
	y = (-lat + 90) * (height/180);

	const dot = document.createElement('div');
	dot.classList.add('dot');
	dot.style.width = dotSize + 'px';
	dot.style.height = dotSize + 'px';
	dot.style.top = (y - dotSize/2) + 'px';
	dot.style.left = (x - dotSize/2) + 'px';
	return dot;
}

(async () => {
	const req = await fetch('airports.json');
	const airports = await req.json();
	console.log(airports);
	// const airport = airports['KORD'];
	// plot(airport.meta.longitudeDegreeNum, airport.meta.latitudeDegreeNum)
	for (let airport_code in airports) {
		const airport = airports[airport_code];
		const lat = airport.meta.latitudeDegreeNum;
		const long = airport.meta.longitudeDegreeNum;
		const dot = createDot(lat, long)
		dot.addEventListener('mouseover', e => {
			document.getElementById('iata').textContent = airport.airportCodeIata;
			document.getElementById('iaco').textContent = airport.meta.icaoCode;
			document.getElementById('name').textContent = airport.airportName;
			document.getElementById('city').textContent = airport.cityName;
			document.getElementById('country').textContent = airport.countryCode;
			document.getElementById('tz').textContent = airport.meta.timeZoneDesc;
			const tzOff = Intl.DateTimeFormat('en-US', {
				timeZoneName: 'short',
				timeZone: airport.meta.timeZoneDesc,
			}).formatToParts().find((i) => i.type === "timeZoneName").value;
			document.getElementById('tz-off').textContent = tzOff;
			if (lat >= 0) {
				document.getElementById('lat').textContent = lat + '°N';
			} else {
				document.getElementById('lat').textContent = -lat + '°S';
			}
			if (long >= 0) {
				document.getElementById('long').textContent = long + '°E';
			} else {
				document.getElementById('long').textContent = -long + '°W';
			}

			modal.style.display = 'block';
			modal.style.left = e.pageX + 'px';
			modal.style.top = e.pageY + 'px';
		});
		document.getElementById('dots').appendChild(dot);
	}

	// const lat = 49.0127983093;
	// const long = 2.5499999523;
	// plot(long, lat);
})();

